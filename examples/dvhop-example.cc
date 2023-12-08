/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/dvhop-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/wifi-mac-helper.h"
#include "ns3/netanim-module.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace ns3;

/**
 * \brief Test script.
 *
 * This script creates 1-dimensional grid topology and then ping last node from the first one:
 *
 * [10.0.0.1] <-- step --> [10.0.0.2] <-- step --> [10.0.0.3] <-- step --> [10.0.0.4]
 *
 *
 */
class DVHopExample
{
public:
  DVHopExample ();
  /// Configure script parameters, \return true on successful configuration
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /// Report results
  void Report (std::ostream & os);
  void LogLocalizationData();
  std::ofstream& GetLatencyLogFile();

private:
  ///\name parameters
  //\{
  /// Number of nodes
  uint32_t size;
  /// Distance between nodes, meters
  double step;
  /// Simulation time, seconds
  double totalTime;
  /// Write per-device PCAP traces if true
  bool pcap;
  /// Print routes if true
  bool printRoutes;
  
  //\}
  ///\name Node Termination
  //\{
  double nodeDeathRate;  // Node death rate parameter for the Weibull distribution
  std::vector<Ptr<Node>> deadNodes;  // A list to keep track of dead nodes
  //\}


  ///\name network
  //\{
  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;
  std::ofstream latencyLogFile;
  std::ofstream localizationLogFile;
  //\}

private:
  void GetTruePositions();
  Vector GetRealPosition(uint32_t nodeId) const;
  void CreateNodes ();
  void CheckAndStopNodes();
  void SimulateCriticalCondition();
  void SetCriticalCondition();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();
  std::vector<bool> criticalCondition;
bool criticalConditionEnabled;
  void SimulateCriticalCondition(uint32_t nodeId, double criticalTime);
  void SetCriticalCondition(uint32_t nodeId, bool condition);

  void CreateBeacons();
  void PrintLoc();
  double CalculateDistance(const Vector& realPosition, const Vector& estimatedPosition) const {
    return (realPosition - estimatedPosition).GetLength();
  }
  
};

int main (int argc, char **argv)
{
  DVHopExample test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();
  test.Report (std::cout);
  return 0;
}

//-----------------------------------------------------------------------------
DVHopExample::DVHopExample () :
  size (50),
  step (50),
  totalTime (10),
  pcap (true),
  printRoutes (true)
{
  localizationLogFile.open("localization_data.csv");
  localizationLogFile << "Time,Node,RealX,RealY,EstimatedX,EstimatedY,LocalizationError\n";
}

std::ofstream& DVHopExample::GetLatencyLogFile()
{
  return latencyLogFile;
}

bool
DVHopExample::Configure (int argc, char **argv)
{
  // Enable DVHop logs by default. Comment this if too noisy
  LogComponentEnable("DVHopRoutingProtocol", LOG_LEVEL_ALL);

  SeedManager::SetSeed (12345);
  CommandLine cmd;

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
  cmd.AddValue ("size", "Number of nodes.", size);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("step", "Grid step, m", step);

  cmd.Parse (argc, argv);
  return true;
}

void
DVHopExample::Run ()
{
//  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();

  CreateBeacons();

  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  Simulator::Stop (Seconds (totalTime));

  AnimationInterface anim("animation.xml");

  Simulator::Run ();
  LogLocalizationData();
  Simulator::Destroy ();
}

void
DVHopExample::Report (std::ostream &)
{
}

void
DVHopExample::CreateNodes ()
{
  std::cout << "Creating " << (unsigned)size << " nodes " << step << " m apart.\n";
  nodes.Create (size);
  // Name nodes
  for (uint32_t i = 0; i < size; ++i)
    {
      std::ostringstream os;
      os << "node-" << i;
      std::cout << "Creating node: "<< os.str ()<< std::endl ;
      Names::Add (os.str (), nodes.Get (i));
    }
  // Create static grid
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (step),
                                 "DeltaY", DoubleValue (step),
                                 "GridWidth", UintegerValue (10),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
}

void DVHopExample::GetTruePositions() {
  for (uint32_t i = 0; i < size; ++i) {
    Ptr<ConstantPositionMobilityModel> mobility =
        nodes.Get(i)->GetObject<ConstantPositionMobilityModel>();
    Vector pos = mobility->GetPosition();
    std::cout << "Node " << i << " true position: (" << pos.x << ", " << pos.y << ")\n";
  }
}
void
DVHopExample::CreateBeacons()
{
 /* int b1 = rand() % size;
  int b2 = rand() % size;
  while(b2 == b1){
    b2 = rand() % size;
  }
  int b3 = rand() % size;
  while(b3 == b1 || b3 == b2){
    b3 = rand() % size;
  }

  Ptr<Ipv4RoutingProtocol> proto = nodes.Get (8)->GetObject<Ipv4>()->GetRoutingProtocol ();
  Ptr<dvhop::RoutingProtocol> dvhop = DynamicCast<dvhop::RoutingProtocol> (proto);
  dvhop->SetIsBeacon (true);
  dvhop->SetPosition (400, 0);

  proto = nodes.Get (24)->GetObject<Ipv4>()->GetRoutingProtocol ();
  dvhop = DynamicCast<dvhop::RoutingProtocol> (proto);
  dvhop->SetIsBeacon (true);
  dvhop->SetPosition (200, 100);

  proto = nodes.Get (37)->GetObject<Ipv4>()->GetRoutingProtocol ();
  dvhop = DynamicCast<dvhop::RoutingProtocol> (proto);
  dvhop->SetIsBeacon (true);
  dvhop->SetPosition (350, 150);
  */

  int n = 0;
  do {
  std::cout << "Enter the percentage of beacons you want (1 - 12.5%, 2 - 25%, 3 - 50%): ";
  std::cin >> n;
  } while ((n != 1) && (n != 2) && (n != 3));
  std::cout << std::endl;

  int beacnum = 0;
  if(n == 1){
    beacnum = size/8;
  } else if (n == 2){
    beacnum = size/4;
  } else if (n == 3){
    beacnum = size/2;
  }

  std::vector<int> v;
  int randnum;
  for(int i = 0; i < beacnum; i++){
    randnum = rand() % size;
    if(!(std::find(v.begin(), v.end(), randnum) != v.end())){
      v.push_back(randnum);
    } else {
      i--;
    }
  }

  Ptr<Ipv4RoutingProtocol> proto;
  Ptr<dvhop::RoutingProtocol> dvhop;
  double bx;
  double by;

  for(auto x : v){
    proto = nodes.Get(x)->GetObject<Ipv4>()->GetRoutingProtocol();
    dvhop = DynamicCast<dvhop::RoutingProtocol> (proto);
    dvhop->SetIsBeacon(true);

    by = std::floor(((x)/10) * 50);
    bx = ((x) % 10) * 50;
        
    dvhop->SetPosition(bx, by);
  }

}


void
DVHopExample::CreateDevices ()
{
  WifiMacHelper wifiMac = WifiMacHelper();
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi = WifiHelper();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  if (pcap)
    {
      wifiPhy.EnablePcapAll (std::string ("aodv"));
    }
}

void DVHopExample::LogLocalizationData() {
  for (uint32_t i = 0; i < size; ++i) {
    Ptr<Ipv4RoutingProtocol> proto = nodes.Get(i)->GetObject<Ipv4>()->GetRoutingProtocol();
    Ptr<dvhop::RoutingProtocol> dvhop = DynamicCast<dvhop::RoutingProtocol>(proto);

    Vector realPosition = dvhop->GetRealPosition();
    Vector estimatedPosition = dvhop->GetPosition();
    double localizationError = CalculateDistance(realPosition, estimatedPosition);

    // Log the data to a file
    localizationLogFile << Simulator::Now().GetSeconds() << ","
                        << i << ","
                        << realPosition.x << "," << realPosition.y << ","
                        << estimatedPosition.x << "," << estimatedPosition.y << ","
                        << localizationError << "\n";
  }
}


void
DVHopExample::InstallInternetStack ()
{
  DVHopHelper dvhop;
  // you can configure DVhop attributes here using aodv.Set(name, value)
  InternetStackHelper stack;
  stack.SetRoutingHelper (dvhop); // has effect on the next Install ()
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);

  Ptr<OutputStreamWrapper> distStream = Create<OutputStreamWrapper>("dvhop.distances", std::ios::out);
  dvhop.PrintDistanceTableAllAt(Seconds(9), distStream);

  if (printRoutes)
    {
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dvhop.routes", std::ios::out);
      dvhop.PrintRoutingTableAllAt (Seconds (8), routingStream);
    }
}

void DVHopExample::SetCriticalCondition(uint32_t nodeId, bool condition)
{
  this->criticalCondition[nodeId] = condition;
}

void DVHopExample::SimulateCriticalCondition(uint32_t nodeId, double criticalTime)
{
 std::cout << "Node " << nodeId << " will enter critical condition at time " << criticalTime << " s\n";

  // Calculate the number of time steps needed to reach criticalTime
  uint32_t stepsToCriticalTime = static_cast<uint32_t>(criticalTime);

  for (uint32_t step = 0; step <= stepsToCriticalTime; ++step)
  {
    CheckAndStopNodes();
    Simulator::Schedule(Seconds(1.0), &DVHopExample::CheckAndStopNodes, this);
  }

  // Set critical condition for the specified node
  SetCriticalCondition(nodeId, true);
}


void DVHopExample::CheckAndStopNodes()
{
  for (uint32_t i = 0; i < size; ++i)
  {
    if (this->criticalCondition[i])
    {
      std::cout << "Node " << i << " is in critical condition. Stopping...\n";
      this->nodes.Get(i)->GetObject<MobilityModel>()->SetPosition(Vector(0, 0, 0));
      this->criticalCondition[i] = false; // Reset critical condition after stopping the node
    }
  }
}

void DVHopExample::HandleNodeDeath(Ptr<Node> node)
{
  std::cout << "Node " << node->GetId() << " has died." << std::endl;

  // Remove the node from the simulation
  node->Dispose ();

  // Add the dead node to the list of dead nodes
  deadNodes.push_back(node);
}



void DVHopExample::PrintLoc(){
  


}