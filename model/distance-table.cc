#include "distance-table.h"
#include "ns3/simulator.h"
#include <algorithm>

namespace ns3
{
	namespace dvhop
	{
		// Constructor for the DistanceTable class
		DistanceTable::DistanceTable()
		{
		}

		// Get the number of hops to a given beacon
		uint16_t
		DistanceTable::GetHopsTo (Ipv4Address beacon) const
		{
			std::map<Ipv4Address, BeaconInfo>::const_iterator it = m_table.find (beacon);
			if( it != m_table.end ())
				{
					return ((BeaconInfo)it->second).GetHops ();
				}

			else return 0;
		}

		// Get the position of a given beacon
		Position
		DistanceTable::GetBeaconPosition (Ipv4Address beacon) const
		{
			std::map<Ipv4Address, BeaconInfo>::const_iterator it = m_table.find (beacon);
			if( it != m_table.end ())
				{
					return ((BeaconInfo)it->second).GetPosition ();
				}

			else return std::make_pair<double,double>(-1.0,-1.0);
		}

		// Add a new beacon to the distance table
		void
		DistanceTable::AddBeacon (Ipv4Address beacon, uint16_t hops, double xPos, double yPos)
		{
			std::map<Ipv4Address, BeaconInfo>::iterator it = m_table.find (beacon);
			BeaconInfo info;
			if( it != m_table.end ())
				{
					info.SetPosition (it->second.GetPosition ());
					info.SetHops (hops);
					info.SetTime (Simulator::Now ());
					it->second = info;
				}
			else
				{
			Position temp;
			temp.first = xPos;
				temp.second = yPos;
					info.SetHops (hops);
					info.SetPosition (temp);
					info.SetTime (Simulator::Now ());
			std::pair<Ipv4Address, BeaconInfo> temp2;
			temp2.first = beacon;
			temp2.second = info;
					m_table.insert (temp2);
				}
		}

		// Get the time when the distance table was last updated for a given beacon
		Time
		DistanceTable::LastUpdatedAt (Ipv4Address beacon) const
		{
			std::map<Ipv4Address, BeaconInfo>::const_iterator it = m_table.find (beacon);
			if( it != m_table.end ())
				{
					return ((BeaconInfo)it->second).GetTime ();
				}

			else return Time::Max ();
		}

		// Get a vector of all known beacons in the distance table
		std::vector<Ipv4Address>
		DistanceTable::GetKnownBeacons() const
		{
			std::vector<Ipv4Address> theBeacons;
			for(std::map<Ipv4Address, BeaconInfo>::const_iterator j = m_table.begin (); j != m_table.end (); ++j)
				{
					theBeacons.push_back (j->first);
				}
			return theBeacons;
		}

		// Print the contents of the distance table to an output stream
		void
		DistanceTable::Print (Ptr<OutputStreamWrapper> os) const
		{
			*os->GetStream () << m_table.size () << " entries\n";
			for(std::map<Ipv4Address,BeaconInfo>::const_iterator j = m_table.begin (); j != m_table.end (); ++j)
				{
					//                    BeaconAddr           BeaconInfo
					*os->GetStream () <<  j->first << "\t" << j->second;
				}
		}

		// Overload the << operator to print a BeaconInfo object to an output stream
		std::ostream &
		operator<< (std::ostream &os, BeaconInfo const &h)
		{
			std::pair<float,float> pos = h.GetPosition ();
			os << h.GetHops () << "\t(" << pos.first << ","<< pos.second << ")\t"<< h.GetTime ()<<"\n";
			return os;
		}

	}
}