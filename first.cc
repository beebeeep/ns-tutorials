/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/simulator-module.h"
#include "ns3/node-module.h"
#include "ns3/helper-module.h"
#include "ns3/ipv4-global-routing-helper.h"


// Default Network Topology
// //
// //       10.1.1.0
// // n0 -------------- n1   n2   n3   n4
// //    point-to-point  |    |    |    |
// //                    ================
// //                      LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("second");

int main (int argc, char *argv[])
{
	uint32_t nPackets = 1, nCsma = 3;
	bool verbose = false;

	CommandLine cmd;
	
	cmd.AddValue("nPackets", "Numbers of packets to echo", nPackets);
	cmd.AddValue("verbose", "Log echo application", verbose);
	cmd.AddValue("nCsma", "Number of CSMA nodes", nCsma);
	cmd.Parse(argc, argv);

	if(nCsma < 1) nCsma = 1;

	if(verbose) {
		LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
		LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
	}

	
	NS_LOG_INFO("Creating topology");
	

	NodeContainer p2pNodes;
	p2pNodes.Create (2);

	NodeContainer csmaNodes;
	csmaNodes.Add(p2pNodes.Get(1));
	csmaNodes.Create(nCsma);

	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
	csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

	NetDeviceContainer p2pDevices;
	p2pDevices = pointToPoint.Install (p2pNodes);

	NetDeviceContainer csmaDevices;
	csmaDevices = csma.Install(csmaNodes);

	InternetStackHelper stack;
	stack.Install(p2pNodes.Get(0));
	stack.Install(csmaNodes);

	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer p2pInterfaces = address.Assign (p2pDevices);
	address.SetBase("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer CsmaInterfaces = address.Assign(csmaDevices);



	UdpEchoServerHelper echoServer (9);

	ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(nCsma));
	serverApps.Start (Seconds (1.0));
	serverApps.Stop (Seconds (10.0));

	UdpEchoClientHelper echoClient (CsmaInterfaces.GetAddress(nCsma), 9);
	echoClient.SetAttribute ("MaxPackets", UintegerValue (nPackets));
	echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.)));
	echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

	ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));
	clientApps.Start (Seconds (2.0));
	clientApps.Stop (Seconds (100.0));

	pointToPoint.EnablePcapAll("second");
	csma.EnablePcap("second", csmaDevices.Get(1), true); 

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();
	
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
