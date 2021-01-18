/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;


void TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  nodeA->GetObject<Ipv4> ()->SetDown (interfaceA);
  nodeB->GetObject<Ipv4> ()->SetDown (interfaceB);
}


int main (int argc, char *argv[])
{
  
  Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::POISON_REVERSE));

  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("1ms"));



/* #region Declaring Nodes */

  NodeContainer net1;
  NodeContainer net2;
  NodeContainer net5;
  NodeContainer net6;
  net1.Create(3);
  net2.Create(4);
  net5.Create(2);
  net6.Create(2);

  Names::Add ("A", net1.Get(0));
  Names::Add ("B", net1.Get(1));
  Names::Add ("C", net1.Get(2));

  Names::Add ("D", net2.Get(0));
  Names::Add ("E", net2.Get(1));
  Names::Add ("F", net2.Get(2));
  Names::Add ("G", net2.Get(3));

  Names::Add ("H", net5.Get(0));
  Names::Add ("I", net5.Get(1));

  Names::Add ("J", net6.Get(0));
  Names::Add ("K", net6.Get(1));


  NodeContainer routers;
  routers.Create(6);

  Names::Add ("R1", routers.Get(0));
  Names::Add ("R2", routers.Get(1));
  Names::Add ("R3", routers.Get(2));
  Names::Add ("R4", routers.Get(3));
  Names::Add ("R5", routers.Get(4));
  Names::Add ("R6", routers.Get(5));


  NodeContainer enddevices;
  enddevices.Add(net1);
  enddevices.Add(net2);
  enddevices.Add(net5);
  enddevices.Add(net6);

/* #endregion */


/* #region Creating Router to Node channels */

    //R1-A
  NodeContainer r1A(routers.Get(0), net1.Get(0));
  NetDeviceContainer netR1_A = csma.Install (r1A);
    //R1-B
  NodeContainer r1B(routers.Get(0), net1.Get(1));
  NetDeviceContainer netR1_B = csma.Install (r1B);
    //R1-C
  NodeContainer r1C(routers.Get(0), net1.Get(2));
  NetDeviceContainer netR1_C = csma.Install (r1C);


    //R2-D
  NodeContainer r2D(routers.Get(1), net2.Get(0));
  NetDeviceContainer netR2_D = csma.Install (r2D);
    //R2-E
  NodeContainer r2E(routers.Get(1), net2.Get(1));
  NetDeviceContainer netR2_E = csma.Install (r2E);
    //R2-F
  NodeContainer r2F(routers.Get(1), net2.Get(2));
  NetDeviceContainer netR2_F = csma.Install (r2F);
    //R2-G
  NodeContainer r2G(routers.Get(1), net2.Get(3));
  NetDeviceContainer netR2_G = csma.Install (r2G);


    //R5-H
  NodeContainer r5H(routers.Get(4), net5.Get(0));
  NetDeviceContainer netR5_H = csma.Install (r5H);
    //R5-I
  NodeContainer r5I(routers.Get(4), net5.Get(1));
  NetDeviceContainer netR5_I = csma.Install (r5I);


    //R6-J
  NodeContainer r6J(routers.Get(5), net6.Get(0));
  NetDeviceContainer netR6_J = csma.Install (r6J);
    //R6-K
  NodeContainer r6K(routers.Get(5), net6.Get(1));
  NetDeviceContainer netR6_K = csma.Install (r6K);

/* #endregion */


/* #region Inter-router channels */

  // R1-R3
  NodeContainer routers13( routers.Get(0),routers.Get(2) );
  NetDeviceContainer netR1_R3 = csma.Install (routers13);

  // R2-R4
  NodeContainer routers24( routers.Get(1),routers.Get(3) );
  NetDeviceContainer netR2_R4 = csma.Install (routers24);

  // R3-R4
  NodeContainer routers34( routers.Get(2),routers.Get(3) );
  NetDeviceContainer netR3_R4 = csma.Install (routers34);

  // R3-R5
  NodeContainer routers35( routers.Get(2),routers.Get(4) );
  NetDeviceContainer netR3_R5 = csma.Install (routers35);

  // R4-R5
  NodeContainer routers45( routers.Get(3),routers.Get(4) );
  NetDeviceContainer netR4_R5 = csma.Install (routers45);

  // R4-R6
  NodeContainer routers46( routers.Get(3),routers.Get(5) );
  NetDeviceContainer netR4_R6 = csma.Install (routers46);

/* #endregion */


/* #region Configuring RIP */

  RipHelper ripRouting;

  //Excluding the host connected interfaces
  for(int i=1; i<=3; i++)
    ripRouting.ExcludeInterface(routers.Get(0), i);
  for(int i=1; i<=4; i++)
    ripRouting.ExcludeInterface(routers.Get(1), i);
  for(int i=1; i<=2; i++)
    ripRouting.ExcludeInterface(routers.Get(4), i);
  for(int i=1; i<=2; i++)
    ripRouting.ExcludeInterface(routers.Get(5), i);

  //R4-R5 cost=5
  ripRouting.SetInterfaceMetric (routers.Get(3), 3, 5);
  ripRouting.SetInterfaceMetric (routers.Get(4), 4, 5);

  Ipv4ListRoutingHelper listRH;
  listRH.Add (ripRouting, 0);

  InternetStackHelper internet;
  internet.SetIpv6StackInstall (false);
  internet.SetRoutingHelper (listRH);
  internet.Install (routers);


  InternetStackHelper internetNodes;
  internetNodes.SetIpv6StackInstall (false);
  internetNodes.Install(enddevices);

/* #endregion */


/* #region Assigning IP addresses */

  Ipv4AddressHelper address;
  
  //For R1-A
  address.SetBase ("192.168.1.0", "255.255.255.192");
  Ipv4InterfaceContainer netR1_Ainterfaces = address.Assign(netR1_A);
  //For R1-B
  address.SetBase ("192.168.1.64", "255.255.255.192");
  Ipv4InterfaceContainer netR1_Binterfaces = address.Assign(netR1_B);
  //For R1-C
  address.SetBase ("192.168.1.128", "255.255.255.192");
  Ipv4InterfaceContainer netR1_Cinterfaces = address.Assign(netR1_C);
  
  //For R2-D
  address.SetBase ("192.168.2.0", "255.255.255.192");
  Ipv4InterfaceContainer netR2_Dinterfaces = address.Assign(netR2_D);
  //For R2-E
  address.SetBase ("192.168.2.64", "255.255.255.192");
  Ipv4InterfaceContainer netR2_Einterfaces = address.Assign(netR2_E);
  //For R2-F
  address.SetBase ("192.168.2.128", "255.255.255.192");
  Ipv4InterfaceContainer netR2_Finterfaces = address.Assign(netR2_F);
  //For R2-G
  address.SetBase ("192.168.2.192", "255.255.255.192");
  Ipv4InterfaceContainer netR2_Ginterfaces = address.Assign(netR2_G);
  
  //For R5-H
  address.SetBase ("192.168.5.0", "255.255.255.128");
  Ipv4InterfaceContainer netR5_Hinterfaces = address.Assign(netR5_H);
  //For R5-I
  address.SetBase ("192.168.5.128", "255.255.255.128");
  Ipv4InterfaceContainer netR5_Iinterfaces = address.Assign(netR5_I);

  //For R6-J
  address.SetBase ("192.168.6.0", "255.255.255.128");
  Ipv4InterfaceContainer netR6_Jinterfaces = address.Assign(netR6_J);
  //For R6-K
  address.SetBase ("192.168.6.128", "255.255.255.128");
  Ipv4InterfaceContainer netR6_Kinterfaces = address.Assign(netR6_K);
  
  
  //For R1-R3
  address.SetBase ("10.1.13.0", "255.255.255.0");
  Ipv4InterfaceContainer router13Interfaces = address.Assign (netR1_R3);
  
  //For R2-R4
  address.SetBase ("10.1.24.0", "255.255.255.0");
  Ipv4InterfaceContainer router24Interfaces = address.Assign (netR2_R4);

  //For R3-R4
  address.SetBase ("10.1.34.0", "255.255.255.0");
  Ipv4InterfaceContainer router34Interfaces = address.Assign (netR3_R4);

  //For R3-R5
  address.SetBase ("10.1.35.0", "255.255.255.0");
  Ipv4InterfaceContainer router35Interfaces = address.Assign (netR3_R5);

  //For R4-R5
  address.SetBase ("10.1.45.0", "255.255.255.0");
  Ipv4InterfaceContainer router45Interfaces = address.Assign (netR4_R5);

  //For R4-R6
  address.SetBase ("10.1.46.0", "255.255.255.0");
  Ipv4InterfaceContainer router46Interfaces = address.Assign (netR4_R6);

/* #endregion */

 
/* #region Configuring Static Routes */

  Ptr<Ipv4StaticRouting> staticRouting;

  // R1-A
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (net1.Get(0)->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute (netR1_Ainterfaces.GetAddress(0), 1);
  // R1-B
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (net1.Get(1)->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute (netR1_Binterfaces.GetAddress(0), 1);
  // R1-C
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (net1.Get(2)->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute (netR1_Cinterfaces.GetAddress(0), 1);
  
  // R2-D
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (net2.Get(0)->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute (netR2_Dinterfaces.GetAddress(0), 1);
  // R2-E
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (net2.Get(1)->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute (netR2_Einterfaces.GetAddress(0), 1);
  // R2-F
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (net2.Get(2)->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute (netR2_Finterfaces.GetAddress(0), 1);
  // R2-G
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (net2.Get(3)->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute (netR2_Ginterfaces.GetAddress(0), 1);
  
  // R5-H
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (net5.Get(0)->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute (netR5_Hinterfaces.GetAddress(0), 1);
  // R5-I
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (net5.Get(1)->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute (netR5_Iinterfaces.GetAddress(0), 1);
  
  // R6-J
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (net6.Get(0)->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute (netR6_Jinterfaces.GetAddress(0), 1);
  // R6-K
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (net6.Get(1)->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute (netR6_Kinterfaces.GetAddress(0), 1);
  
/* #endregion */


//Printing Routing Tables
  RipHelper routingHelper;

  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);

  for(int i=0; i<6; i++)
  {
    routingHelper.PrintRoutingTableAt (Seconds (10.0), routers.Get(i), routingStream);
    routingHelper.PrintRoutingTableAt (Seconds (40.0), routers.Get(i), routingStream);
    routingHelper.PrintRoutingTableAt (Seconds (80.0), routers.Get(i), routingStream);
  }
//


//Ping commands
  bool pingVerbose = true;
  //A pings K
  V4PingHelper ping1 (netR6_Kinterfaces.GetAddress (1));
  ping1.SetAttribute ("Interval", TimeValue (Seconds(1.0)));
  ping1.SetAttribute ("Size", UintegerValue (1024));
  ping1.SetAttribute ("Verbose", BooleanValue (pingVerbose));
  ApplicationContainer ping1app = ping1.Install (net1.Get(0));
  ping1app.Start (Seconds (1.0));
  ping1app.Stop (Seconds (81.0));

  //G pings H
  V4PingHelper ping2 (netR5_Hinterfaces.GetAddress(1));
  ping2.SetAttribute ("Interval", TimeValue (Seconds(1.0)));
  ping2.SetAttribute ("Size", UintegerValue (1024));
  ping2.SetAttribute ("Verbose", BooleanValue (pingVerbose));
  ApplicationContainer ping2app = ping2.Install (net2.Get(3));
  ping2app.Start (Seconds (1.0));
  ping2app.Stop (Seconds (81.0));
//


//Network traces
  // AsciiTraceHelper ascii;
  // csma.EnableAsciiAll (ascii.CreateFileStream ("CN-A4.pcap"));
  // csma.EnablePcapAll ("CN-A4", true);
  NodeContainer show_pcaps(net1.Get(0), net2.Get(3), net5.Get(0), net6.Get(1));
  csma.EnablePcap("A4-capture",show_pcaps);
//  



//Break R3-R4 link at 25s and stop simulation at 90s
  Simulator::Schedule(Seconds(25), &TearDownLink, routers34.Get(0), routers34.Get(1), 2, 2);
  Simulator::Stop (Seconds (90));
//



/* #region Setting Up Animation */

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(routers);
  mobility.Install(enddevices);

  AnimationInterface anim("animation.xml");

  anim.SetConstantPosition(net1.Get(0), 0.0, 0.0);
  anim.SetConstantPosition(net1.Get(1), 10.0, 0.0);
  anim.SetConstantPosition(net1.Get(2), 20.0, 0.0);
  anim.SetConstantPosition(routers.Get(0), 10.0, 10.0);

  anim.SetConstantPosition(net2.Get(0), 35.0, 0.0);
  anim.SetConstantPosition(net2.Get(1), 45.0, 0.0);
  anim.SetConstantPosition(net2.Get(2), 55.0, 0.0);
  anim.SetConstantPosition(net2.Get(3), 65.0, 0.0);
  anim.SetConstantPosition(routers.Get(1), 50.0, 10.0);

  anim.SetConstantPosition(routers.Get(2), 20.0, 20.0);
  anim.SetConstantPosition(routers.Get(3), 40.0, 20.0);
  
  anim.SetConstantPosition(net5.Get(0), 0.0, 40.0);
  anim.SetConstantPosition(net5.Get(1), 20.0, 40.0);
  anim.SetConstantPosition(routers.Get(4), 10.0, 30.0);

  anim.SetConstantPosition(net6.Get(0), 40.0, 40.0);
  anim.SetConstantPosition(net6.Get(1), 60.0, 40.0);
  anim.SetConstantPosition(routers.Get(5), 50.0, 30.0);

  anim.UpdateNodeDescription(0, "A");
  anim.UpdateNodeDescription(1, "B");
  anim.UpdateNodeDescription(2, "C");
  
  anim.UpdateNodeDescription(3, "D");
  anim.UpdateNodeDescription(4, "E");
  anim.UpdateNodeDescription(5, "F");
  anim.UpdateNodeDescription(6, "G");
  
  anim.UpdateNodeDescription(7, "H");
  anim.UpdateNodeDescription(8, "I");
  
  anim.UpdateNodeDescription(9, "J");
  anim.UpdateNodeDescription(10, "K");
  
  anim.UpdateNodeDescription(11, "R1");
  anim.UpdateNodeDescription(12, "R2");
  anim.UpdateNodeDescription(13, "R3");
  anim.UpdateNodeDescription(14, "R4");
  anim.UpdateNodeDescription(15, "R5");
  anim.UpdateNodeDescription(16, "R6");

/* #endregion */


//Start the simulation
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
