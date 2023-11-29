#include "dvhop-packet.h"
#include "ns3/packet.h"
#include "ns3/address-utils.h"

namespace ns3
{
  namespace dvhop
  {
    // Ensure the FloodingHeader class is registered with ns-3's TypeId system
    NS_OBJECT_ENSURE_REGISTERED (FloodingHeader);

    // Default constructor
    FloodingHeader::FloodingHeader()
    {
    }

    // Constructor with parameters
    FloodingHeader::FloodingHeader(double xPos, double yPos, uint16_t seqNo, uint16_t hopCount, Ipv4Address beacon)
    {
      m_xPos     = xPos;      // X position of the node
      m_yPos     = yPos;      // Y position of the node
      m_seqNo    = seqNo;     // Sequence number of the packet
      m_hopCount = hopCount;  // Hop count from the source
      m_beaconId = beacon;    // IP address of the beacon node
    }

    // Get the TypeId of the class
    TypeId
    FloodingHeader::GetTypeId ()
    {
      static TypeId tid = TypeId("ns3::dvhop::FloodingHeader")
          .SetParent<Header> ()
          .AddConstructor<FloodingHeader>();
      return tid;
    }

    TypeId
    FloodingHeader::GetInstanceTypeId () const
    {
      return GetTypeId ();
    }

    // Get the serialized size of the header
    uint32_t
    FloodingHeader::GetSerializedSize () const
    {
      return 24; //Total number of bytes when serialized
    }

    // Serialize the header to a buffer
    void
    FloodingHeader::Serialize (Buffer::Iterator start) const
    {
      // Serialize the X and Y positions as uint64_t
      double x = m_xPos;
      uint64_t dst;
      char *const p = reinterpret_cast<char*>(&x);
      std::copy(p, p+sizeof(uint64_t), reinterpret_cast<char*>(&dst));
      start.WriteHtonU64 (dst);

      double y = m_yPos;
      char* const p2 = reinterpret_cast<char*>(&y);
      std::copy(p2, p2+sizeof(uint64_t), reinterpret_cast<char*>(&dst));
      start.WriteHtonU64 (dst);

      // Serialize the sequence number, hop count, and beacon ID
      start.WriteU16 (m_seqNo);
      start.WriteU16 (m_hopCount);
      WriteTo(start, m_beaconId);
    }

    // Deserialize the header from a buffer
    uint32_t
    FloodingHeader::Deserialize (Buffer::Iterator start)
    {
      Buffer::Iterator i = start;

      // Deserialize the X and Y positions, sequence number, hop count, and beacon ID
      uint64_t midX = i.ReadNtohU64 ();
      char *const p = reinterpret_cast<char*>(&midX);
      std::copy(p, p + sizeof(double), reinterpret_cast<char*>(&m_xPos));

      uint64_t midY = i.ReadNtohU64 ();
      char* const p2 = reinterpret_cast<char*>(&midY);
      std::copy(p2, p2 + sizeof(double), reinterpret_cast<char*>(&m_yPos));


      std::cout << "Deserializing coordinates ("<<m_xPos <<","<<m_yPos<<")"<<std::endl;

      m_seqNo = i.ReadU16 ();
      m_hopCount = i.ReadU16 ();
      ReadFrom (i, m_beaconId);

      //Validate the readed bytes match the serialized size
      uint32_t dist = i.GetDistanceFrom (start);
      NS_ASSERT (dist == GetSerializedSize () );
      return dist;
    }

    // Print the header to an output stream
    void
    FloodingHeader::Print (std::ostream &os) const
    {
      os << "Beacon: " << m_beaconId << " ,hopCount: " << m_hopCount << ", (" << m_xPos << ", "<< m_yPos<< ")\n";

    }

    // Overload the << operator to print the header
    std::ostream &
    operator<< (std::ostream &os, FloodingHeader const &h)
    {
      h.Print (os);
      return os;
    }
  }
}
