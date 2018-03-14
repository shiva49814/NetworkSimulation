#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-apps-module.h"
#include <fstream>
#include <iostream>
#include <string>
#include <cassert>
#include <cstring>

using namespace ns3;
//Ping functions----------------------------------------------------------------------------------------------------------------------------------------------- 
//NS_LOG_COMPONENT_DEFINE ("CsmaPingExample");
    
    static void SinkRx (Ptr<const Packet> p, const Address &ad)
    {
      //std::cout << *p << std::endl;
    }
    
    static void PingRtt (std::string context, Time rtt)
    {
      //std::cout << context << " " << rtt << std::endl;
    }




//Packet sink application-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class MyApp : public Application 
{
public:

  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0), 
    m_peer (), 
    m_packetSize (0), 
    m_nPackets (0), 
    m_dataRate (0), 
    m_sendEvent (), 
    m_running (false), 
    m_packetsSent (0)
{
}

MyApp::~MyApp()
{
  m_socket = 0;
}

void MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

static void CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
}

static void RxDrop (Ptr<const Packet> p)
{
  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
}



// Network Topology
//
// 
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//





NS_LOG_COMPONENT_DEFINE ("Project");

int main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 15;
  uint32_t nWifi = 15;
  bool tracing = true;

  std::string transportProt = "Tcp";
  std::string socketType;

	CommandLine cmd;
	cmd.AddValue ("transportProt", "Transport protocol to use: Tcp, Udp", transportProt);
	cmd.Parse (argc, argv);

	if (transportProt.compare ("Tcp") == 0)
	{
		socketType = "ns3::TcpSocketFactory";
	}
	else
	{
		socketType = "ns3::UdpSocketFactory";
	}
  //std::cout<<socketType<<std::endl;

  //CommandLine cmd;
  //cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  //cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  //cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
  //Creating all the nodes for network.-------------------------------------------------------------------------------------------------------------------------------------------------------------------
  NodeContainer routers;
  NodeContainer csmaNode1;
  NodeContainer csmaNode2;
  NodeContainer wifiStaNodes1;
  NodeContainer wifiStaNodes2;
  
  routers.Create (5);
  csmaNode1.Create(nCsma);
  csmaNode2.Create(nCsma);
  wifiStaNodes1.Create(nWifi);
  wifiStaNodes2.Create(nWifi);
        
  	if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }
 
  //Installing stack on all the nodes.-------------------------------------------------------------------------------------------------------------------------------------------------------------------
  InternetStackHelper stack;
	stack.Install(routers);
	stack.Install(csmaNode1);
        stack.Install(csmaNode2);
	stack.Install(wifiStaNodes1);
	stack.Install(wifiStaNodes2);
  

  //Creating all helper classes.-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //Create Csma helper
	CsmaHelper csma;
	
	//Create Point-to-Point helper
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
		
        //Create an Address helper
	Ipv4AddressHelper address;

  //Connecting all the routers to each other via point to point to connections.------------------------------------------------------------------------------------------------------------------------
   
        //connecting router 1 and 2----------------------------------------------------------------------
	/*NodeContainer router12;
	router12.Add(routers.Get(1));
	router12.Add(routers.Get(2));
	
	//Create a device container to hold net device installed on each node
	NetDeviceContainer router12Devices = p2p.Install(router12);
	
	//Configure the subnet address and mask
	address.SetBase("10.1.1.0","255.255.255.0");
	
	//Create and interface conatiner to hold the ipv4 interfaces created and assign IP address to each interface
	Ipv4InterfaceContainer router12Interfaces = address.Assign(router12Devices);
	
        
        //connecting router 2 and 3----------------------------------------------------------------------------
	NodeContainer router23;
	router23.Add(routers.Get(2));
	router23.Add(routers.Get(3));
	
	//Create a device container to hold net device installed on each node
	NetDeviceContainer router23Devices = p2p.Install(router23);
	
	//Configure the subnet address and mask
	address.SetBase("10.1.2.0","255.255.255.0");
	
	//Create and interface conatiner to hold the ipv4 interfaces created and assign IP address to each interface
	Ipv4InterfaceContainer router23Interfaces = address.Assign(router23Devices);
	
      

         //connecting router 3 and 4-----------------------------------------------------------------------------
	NodeContainer router34;
	router34.Add(routers.Get(3));
	router34.Add(routers.Get(4));
	
	//Create a device container to hold net device installed on each node
	NetDeviceContainer router34Devices = p2p.Install(router34);
	
	//Configure the subnet address and mask
	address.SetBase("10.1.3.0","255.255.255.0");
	
	//Create and interface conatiner to hold the ipv4 interfaces created and assign IP address to each interface
	Ipv4InterfaceContainer router34Interfaces = address.Assign(router34Devices);
	
         
         //connecting router 1 and 4----------------------------------------------------------------------------
	NodeContainer router14;
	router14.Add(routers.Get(1));
	router14.Add(routers.Get(4));
	
	//Create a device container to hold net device installed on each node
	NetDeviceContainer router14Devices = p2p.Install(router14);
	
	//Configure the subnet address and mask
	address.SetBase("10.1.4.0","255.255.255.0");
	
	//Create and interface conatiner to hold the ipv4 interfaces created and assign IP address to each interface
	Ipv4InterfaceContainer router14Interfaces = address.Assign(router14Devices);*/

        
        //connecting router 1 and 0--------------------------------------------------------------------------------
	
	NodeContainer router10;
	router10.Add(routers.Get(1));
	router10.Add(routers.Get(0));
	
	//Create a device container to hold net device installed on each node
	NetDeviceContainer router10Devices = p2p.Install(router10);
	
	//Configure the subnet address and mask
	address.SetBase("10.1.5.0","255.255.255.0");
	
	//Create and interface conatiner to hold the ipv4 interfaces created and assign IP address to each interface
	Ipv4InterfaceContainer router10Interfaces = address.Assign(router10Devices);

   
         //connecting router 2 and 0---------------------------------------------------------------------------
	
	NodeContainer router20;
	router20.Add(routers.Get(2));
	router20.Add(routers.Get(0));
	
	//Create a device container to hold net device installed on each node
	NetDeviceContainer router20Devices = p2p.Install(router20);
	
	//Configure the subnet address and mask
	address.SetBase("10.1.6.0","255.255.255.0");
	
	//Create and interface conatiner to hold the ipv4 interfaces created and assign IP address to each interface
	Ipv4InterfaceContainer router20Interfaces = address.Assign(router20Devices);
	

        //connecting router 3 and 0------------------------------------------------------------------------
	
	NodeContainer router30;
	router30.Add(routers.Get(3));
	router30.Add(routers.Get(0));
	
	//Create a device container to hold net device installed on each node
	NetDeviceContainer router30Devices = p2p.Install(router30);
	
	//Configure the subnet address and mask
	address.SetBase("10.1.7.0","255.255.255.0");
	
	//Create and interface conatiner to hold the ipv4 interfaces created and assign IP address to each interface
	Ipv4InterfaceContainer router30Interfaces = address.Assign(router30Devices);
	        
        
         //connecting router 4 and 0-----------------------------------------------------------------------
	
	NodeContainer router40;
	router40.Add(routers.Get(4));
	router40.Add(routers.Get(0));
	
	//Create a device container to hold net device installed on each node
	NetDeviceContainer router40Devices = p2p.Install(router40);
	
	//Configure the subnet address and mask
	address.SetBase("10.1.8.0","255.255.255.0");
	
	//Create and interface conatiner to hold the ipv4 interfaces created and assign IP address to each interface
	Ipv4InterfaceContainer router40Interfaces = address.Assign(router40Devices);
	


  //Connecing wifiStaNodes1 to router1-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
        	//Create a Node conatainer to hold nodes that belong to subnet 6 and add branch 0 and wifi nodes
	NodeContainer wifisubnet1;
	NodeContainer wifiApNode1 = routers.Get(1);
	wifisubnet1.Add(wifiStaNodes1);
	
	YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default ();
	YansWifiPhyHelper phy1 = YansWifiPhyHelper::Default ();
	phy1.SetChannel (channel1.Create ());
	
	WifiHelper wifi1;
	wifi1.SetRemoteStationManager ("ns3::AarfWifiManager");

	WifiMacHelper mac1;
	Ssid ssid1 = Ssid ("ns-3-ssid");
	mac1.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid1),
               "ActiveProbing", BooleanValue (false));

	NetDeviceContainer staDevices1;
	staDevices1 = wifi1.Install (phy1, mac1, wifiStaNodes1);

	mac1.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid1));

	NetDeviceContainer apDevices1;
	apDevices1 = wifi1.Install (phy1, mac1, wifiApNode1);

	MobilityHelper mobility1;

	mobility1.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (28.0),
                                 "MinY", DoubleValue (8.0),
                                 "DeltaX", DoubleValue (3.0),
                                 "DeltaY", DoubleValue (3.0),
                                 "GridWidth", UintegerValue (8),
                                 "LayoutType", StringValue ("RowFirst"));

	mobility1.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
	mobility1.Install (wifiStaNodes1);

	mobility1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility1.Install (wifiApNode1);
	
	address.SetBase ("182.16.1.0", "255.255.255.0");
	address.Assign (staDevices1);
	address.Assign (apDevices1);

//Connecing wifiStaNodes2 to router3-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
        	//Create a Node conatainer to hold nodes that belong to subnet 6 and add branch 0 and wifi nodes
	NodeContainer wifisubnet2;
	NodeContainer wifiApNode2 = routers.Get(3);
	wifisubnet2.Add(wifiStaNodes2);
	
	YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default ();
	YansWifiPhyHelper phy2 = YansWifiPhyHelper::Default ();
	phy2.SetChannel (channel2.Create ());
	
	WifiHelper wifi2;
	wifi2.SetRemoteStationManager ("ns3::AarfWifiManager");

	WifiMacHelper mac2;
	Ssid ssid2 = Ssid ("ns-3-ssid");
	mac2.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid2),
               "ActiveProbing", BooleanValue (false));

	NetDeviceContainer staDevices2;
	staDevices2 = wifi2.Install (phy2, mac2, wifiStaNodes2);

	mac2.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid2));

	NetDeviceContainer apDevices2;
	apDevices2 = wifi2.Install (phy2, mac2, wifiApNode2);

	MobilityHelper mobility2;

	mobility2.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (28.0),
                                 "MinY", DoubleValue (18.0),
                                 "DeltaX", DoubleValue (3.0),
                                 "DeltaY", DoubleValue (3.0),
                                 "GridWidth", UintegerValue (8),
                                 "LayoutType", StringValue ("RowFirst"));

	mobility2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
	mobility2.Install (wifiStaNodes2);

	mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility2.Install (wifiApNode2);
	
	address.SetBase ("132.16.1.0", "255.255.255.0");
	address.Assign (staDevices2);
	address.Assign (apDevices2);



//Adding Csmanode1 and router2 to CSMA Nodes------------------------------------------------------------------------------------------------------------------------------------------------------
        
        NodeContainer csmaNodes1;
        csmaNodes1.Add (routers.Get (2));
        csmaNodes1.Add(csmaNode1);
        
        
        CsmaHelper csma1;
        csma1.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
        csma1.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

        NetDeviceContainer csmaDevices1;
        csmaDevices1 = csma1.Install (csmaNodes1);
        
        address.SetBase ("20.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer csmaInterfaces1;
        csmaInterfaces1 = address.Assign (csmaDevices1);



//Adding Csmanode2 and router4 to CSMA Nodes-------------------------------------------------------------------------------------------------------------------------------------------------------
        
        NodeContainer csmaNodes2;
        csmaNodes2.Add (routers.Get (4));
        csmaNodes2.Add(csmaNode2);
        
        
        CsmaHelper csma2;
        csma2.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
        csma2.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

        NetDeviceContainer csmaDevices2;
        csmaDevices2 = csma2.Install (csmaNodes2);
        
        address.SetBase ("40.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer csmaInterfaces2;
        csmaInterfaces2 = address.Assign (csmaDevices2);


//UDP ECHO Application---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

        //UDP0 csma1 to wifi1----------------------------------------------------------------------
        UdpEchoServerHelper echoServer (9);

        ApplicationContainer serverApps = echoServer.Install (csmaNodes1.Get (nCsma));
        serverApps.Start (Seconds (1.0));
  	serverApps.Stop (Seconds (10.0));
 
  	UdpEchoClientHelper echoClient (csmaInterfaces1.GetAddress (nCsma), 9);
  	echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  	echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  	echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  	ApplicationContainer clientApps = 
    	echoClient.Install (wifiStaNodes1.Get (nWifi - 1));
  	clientApps.Start (Seconds (2.0));
  	clientApps.Stop (Seconds (10.0));

  	//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  	

  	/*if (tracing == true)
    	{
      	p2p.EnablePcapAll ("third");
      	phy1.EnablePcap ("third", apDevices1.Get (0));
      	csma1.EnablePcap ("third", csmaDevices1.Get (0), true);
    	}*/

       //UDP1 csma2 to wifi2----------------------------------------------------------------------
        UdpEchoServerHelper echoServer1 (9);

        ApplicationContainer serverApps1 = echoServer1.Install (csmaNodes2.Get (nCsma));
        serverApps1.Start (Seconds (1.0));
  	serverApps1.Stop (Seconds (10.0));
 
  	UdpEchoClientHelper echoClient1 (csmaInterfaces2.GetAddress (nCsma), 9);
  	echoClient1.SetAttribute ("MaxPackets", UintegerValue (1));
  	echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  	echoClient1.SetAttribute ("PacketSize", UintegerValue (1024));

  	ApplicationContainer clientApps1 = 
    	echoClient1.Install (wifiStaNodes2.Get (nWifi - 1));
  	clientApps1.Start (Seconds (2.0));
  	clientApps1.Stop (Seconds (10.0));

  	//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  	//Simulator::Stop (Seconds (10.0));

  	

        //UDP1 csma1 to csma1------------------------------------------------------------------------
        UdpEchoServerHelper echoServer11 (9);

        ApplicationContainer serverApps11 = echoServer11.Install (csmaNodes1.Get (11));
        serverApps11.Start (Seconds (1.0));
  	serverApps11.Stop (Seconds (10.0));
 
  	UdpEchoClientHelper echoClient11 (csmaInterfaces1.GetAddress (11), 9);
  	echoClient11.SetAttribute ("MaxPackets", UintegerValue (1));
  	echoClient11.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  	echoClient11.SetAttribute ("PacketSize", UintegerValue (1024));

  	ApplicationContainer clientApps11 = 
    	echoClient11.Install (csmaNodes1.Get (1));
  	clientApps11.Start (Seconds (2.0));
  	clientApps11.Stop (Seconds (10.0));

  	//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  	//Simulator::Stop (Seconds (10.0));

  	

         //UDP1 csma2 to csma2---------------------------------------------------------------------
        UdpEchoServerHelper echoServer22 (9);

        ApplicationContainer serverApps22 = echoServer22.Install (csmaNodes2.Get (11));
        serverApps22.Start (Seconds (1.0));
  	serverApps22.Stop (Seconds (10.0));
 
  	UdpEchoClientHelper echoClient22 (csmaInterfaces2.GetAddress (11), 9);
  	echoClient22.SetAttribute ("MaxPackets", UintegerValue (1));
  	echoClient22.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  	echoClient22.SetAttribute ("PacketSize", UintegerValue (1024));

  	ApplicationContainer clientApps22 = 
    	echoClient22.Install (csmaNodes2.Get (1));
  	clientApps22.Start (Seconds (2.0));
  	clientApps22.Stop (Seconds (10.0));

  	//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  	//Simulator::Stop (Seconds (10.0));

  	if (tracing == true)
    	{
      	p2p.EnablePcapAll ("project");
      	phy2.EnablePcap ("project", apDevices2.Get (0));
      	csma2.EnablePcap ("project", csmaDevices2.Get (0), true);
    	}



       
//ON_OFF Application----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  //csma 1 and csma 2
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  csmaDevices1.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (csmaInterfaces1.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (csmaNodes1.Get (1));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20.));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (csmaNodes2.Get (1), TcpSocketFactory::GetTypeId ());
  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));

  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, 1040, 1000, DataRate ("1Mbps"));
  csmaNodes1.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20.));

  csmaDevices1.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));

   //csma 1 and wifi1--------------------------------------------------------------------------
  Ptr<RateErrorModel> em11 = CreateObject<RateErrorModel> ();
  em11->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  csmaDevices1.Get (3)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  uint16_t sinkPort11 = 8080;
  Address sinkAddress11 (InetSocketAddress (csmaInterfaces1.GetAddress (3), sinkPort));
  PacketSinkHelper packetSinkHelper11 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort11));
  ApplicationContainer sinkApps11 = packetSinkHelper11.Install (csmaNodes1.Get (3));
  sinkApps11.Start (Seconds (0.));
  sinkApps11.Stop (Seconds (20.));

  Ptr<Socket> ns3TcpSocket11 = Socket::CreateSocket (wifisubnet1.Get (3), TcpSocketFactory::GetTypeId ());
  ns3TcpSocket11->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));

  Ptr<MyApp> app11 = CreateObject<MyApp> ();
  app11->Setup (ns3TcpSocket11, sinkAddress11, 1040, 1000, DataRate ("1Mbps"));
  csmaNodes1.Get (0)->AddApplication (app11);
  app11->SetStartTime (Seconds (1.));
  app11->SetStopTime (Seconds (20.));

  csmaDevices1.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));




//PING Application-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
   
   NS_LOG_INFO ("Create Source");
   Config::SetDefault ("ns3::Ipv4RawSocketImpl::Protocol", StringValue ("2"));
   InetSocketAddress dst = InetSocketAddress (csmaInterfaces2.GetAddress (3));
   OnOffHelper onoff = OnOffHelper ("ns3::Ipv4RawSocketFactory", dst);
   onoff.SetConstantRate (DataRate (15000));
   onoff.SetAttribute ("PacketSize", UintegerValue (1200));
   
    
   ApplicationContainer apps = onoff.Install (csmaNodes1.Get (0));
   apps.Start (Seconds (1.0));
   apps.Stop (Seconds (10.0));
   
   NS_LOG_INFO ("Create Sink.");
   PacketSinkHelper sink = PacketSinkHelper ("ns3::Ipv4RawSocketFactory", dst);
   apps = sink.Install (csmaNodes1.Get (3));
   apps.Start (Seconds (0.0));
   apps.Stop (Seconds (11.0));
    
   NS_LOG_INFO ("Create pinger");
   V4PingHelper ping = V4PingHelper (csmaInterfaces1.GetAddress (2));
   NodeContainer pingers;
   pingers.Add (wifisubnet1.Get (1));
   pingers.Add (csmaNodes2.Get (1));
   pingers.Add (wifisubnet2.Get (3));
   pingers.Add (csmaNodes1.Get(5));
   apps = ping.Install (pingers);
   apps.Start (Seconds (2.0));
   apps.Stop (Seconds (5.0));
   
   NS_LOG_INFO ("Configure Tracing.");
   // first, pcap tracing in non-promiscuous mode
   csma.EnablePcapAll ("csma-ping", true);
  
   // then, print what the packet sink receives.
   Config::ConnectWithoutContext ("/NodeList/3/ApplicationList/0/$ns3::PacketSink/Rx", 
   MakeCallback (&SinkRx));
   // finally, print the ping rtts.
   Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::V4Ping/Rtt",
   MakeCallback (&PingRtt));
   
   Packet::EnablePrinting ();

//Routing part------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

NS_LOG_INFO ("Enable static global routing.");
	//
	// Turn on global static routing so we can actually be routed across the star.
	//  
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	
	// Trace routing tables 
	Ipv4GlobalRoutingHelper g;
	Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing.routes", std::ios::out);
	g.PrintRoutingTableAllAt (Seconds (12), routingStream);
	
	Simulator::Stop (Seconds (20.0));
	
	


//NetAnimation part------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      
        MobilityHelper mobility3;
	mobility3.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility3.Install(routers);
	mobility3.Install(csmaNodes1);
	mobility3.Install(csmaNodes2);
	
	AnimationInterface anim ("Project.xml");
	anim.SetConstantPosition(routers.Get(0), 30.0, 20.0);
	anim.SetConstantPosition(routers.Get(1), 15.0, 20.0);
	anim.SetConstantPosition(routers.Get(2), 30.0, 10.0);
	anim.SetConstantPosition(routers.Get(3), 45.0, 20.0);
	anim.SetConstantPosition(routers.Get(4), 30.0, 30.0);

        for(int i = 0; i<15;i++){
	anim.SetConstantPosition(csmaNodes1.Get(i+1), 10.0-i, 20.0);}
 	for(int i = 0; i<15;i++){
	anim.SetConstantPosition(csmaNodes2.Get(i+1), 45.0+i, 20.0);}
 	//for(int i = 0; i<15;i++){
	//anim.SetConstantPosition(wifiStaNodes2.Get(i+1), 45.0+i, 20.0);}
	/*anim.SetConstantPosition(csmaNodes2, 50.0, 20.0);
	anim.SetConstantPosition(wifiStaNodes1, 30.0, 5.0);
	anim.SetConstantPosition(wifiStaNodes2, 30.0, 40.0);*/


        
        Simulator::Run ();
        Simulator::Destroy ();
        return 0;
}

