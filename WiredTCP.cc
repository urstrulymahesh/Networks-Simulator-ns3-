#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include "ns3/gnuplot.h"
#include "ns3/gnuplot-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Assignment 4 Wired TCP");
Ptr<PacketSink> sink;

class App : public Application
{
public:

  App ();
  virtual ~App();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
  void ChangeRate(DataRate newrate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);
	void SendPacket2 (int);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

App::App ()
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

App::~App()
{
  m_socket = 0;
}

void
App::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
App::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
App::StopApplication (void)
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

void
App::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
App::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &App::SendPacket, this);
    }
}

int main()
{
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));

  std :: string plotTitle1               = "Fairness Index Vs Packet Size";
  std :: string dataTitle1               = "Fairness Index Data";

  //.........Fairness Plot Setup ........//
  std :: string fairnessFile            = "wiredfairness";
	std :: string fairnessGraphics        = fairnessFile + ".png";
	std :: string fairnessPlot            = fairnessFile + ".plt";

	Gnuplot plot1 (fairnessGraphics);
	plot1.SetTitle (plotTitle1);
	// Make the graphics file, which the plot file will create when it is used with Gnuplot, be a PNG file.
	plot1.SetTerminal ("png");
	plot1.SetLegend ("Packet Size", "Fairness Index");
	plot1.AppendExtra ("set xrange [40:1500]");
	// Instantiate the dataset, set its title, and make the points be plotted along with connecting lines.
	Gnuplot2dDataset fairnessDataset;
	fairnessDataset.SetTitle (dataTitle1);
  fairnessDataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  std :: string plotTitle2               = "Throughput Vs Packet Size";
  std :: string dataTitle2               = "Throughput Data";

  //.........Throughput Plot Setup..........//
  std :: string throughputFile           = "wiredthroughput";
  std :: string throughputGraphics       = throughputFile + ".png";
  std :: string throughputPlot           = throughputFile + ".plt";

  Gnuplot plot2 (throughputGraphics);
  plot2.SetTitle (plotTitle2);
  // Make the graphics file, which the plot file will create when it is used with Gnuplot, be a PNG file.
  plot2.SetTerminal ("png");
  plot2.SetLegend ("Packet Size", "Throughput");
  plot2.AppendExtra ("set xrange [40:1500]");
  // Instantiate the dataset, set its title, and make the points be plotted along with connecting lines.
  Gnuplot2dDataset throughputDataset;
  throughputDataset.SetTitle (dataTitle2);
  throughputDataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  //Packet Size to be sent
  uint32_t packets[10] = {40,44,48,52,60,552,576,628,1420,1500};
  //Number of packets to be sent
  uint32_t num_packets = 10;
  uint32_t j;
  for(j=0;j<10;j++)
  {
    std::cout << "Packet Size : " << packets[j] << std::endl;
    //Creating  Nodes
    NS_LOG_INFO("Creating Nodes");
    Ptr<Node> n2 = CreateObject<Node> ();
    Ptr<Node> n3 = CreateObject<Node> ();
    Ptr<Node> r1 = CreateObject<Node> ();
    Ptr<Node> r2 = CreateObject<Node> ();

    //Wired TCP Network Setup
    NodeContainer nodeRouterContainer1 = NodeContainer(n2,r1);
    NodeContainer routerContainer = NodeContainer(r1,r2);
    NodeContainer nodeRouterContainer2 = NodeContainer(r2,n3);
    NodeContainer all = NodeContainer(nodeRouterContainer1,nodeRouterContainer2) ;

    //Install Internet Stack
    NS_LOG_INFO("Install Internet Stack");
    InternetStackHelper stack;
    stack.Install(all);

    //Create channels
    NS_LOG_INFO("Create Channels");
    PointToPointHelper p2pHelper1,p2pHelper2;
    p2pHelper1.SetChannelAttribute("Delay",StringValue("20ms"));
    p2pHelper1.SetDeviceAttribute("DataRate",StringValue("100Mbps"));
    NetDeviceContainer nodeRouterDevice1 = p2pHelper1.Install(nodeRouterContainer1);
    NetDeviceContainer nodeRouterDevice2 = p2pHelper1.Install(nodeRouterContainer2);

    //Set Drop Tail Queue on routers r1 and r2
    p2pHelper2.SetQueue("ns3::DropTailQueue","Mode",EnumValue (DropTailQueue::QUEUE_MODE_BYTES),"MaxBytes",UintegerValue (22500));
    p2pHelper2.SetDeviceAttribute("DataRate",StringValue("10Mbps"));
    p2pHelper2.SetChannelAttribute("Delay",StringValue("50ms"));
    NetDeviceContainer routerDevice = p2pHelper2.Install(routerContainer);

    //Assign IPV4 address
    NS_LOG_INFO("Assign IPV4 address");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.3.1.0","255.255.255.0");
    Ipv4InterfaceContainer nodeRouterInterface1 = ipv4.Assign(nodeRouterDevice1);

    ipv4.SetBase("10.3.2.0","255.255.255.0");
    Ipv4InterfaceContainer nodeRouterInterface2 = ipv4.Assign(nodeRouterDevice2);

    ipv4.SetBase("10.3.3.0","255.255.255.0");
    Ipv4InterfaceContainer routerInterface = ipv4.Assign(routerDevice);

    NS_LOG_INFO ("Enable static global routing.");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    std::cout <<"n2 : " << nodeRouterInterface1.GetAddress(0) << "\t";
    std::cout <<"r1 : " << nodeRouterInterface1.GetAddress(1) << "\t";
    std::cout <<"r2 : " << nodeRouterInterface2.GetAddress(0) << "\t";
    std::cout <<"n3 : " << nodeRouterInterface2.GetAddress(1) << std::endl;
    std::cout << std::endl;

    //Wired TCP connection from n2 to n3
    //tcp connection from n2 to n3
    //Install TCP sink on n3
    uint16_t sinkport = 8080;
    Address sinkaddress(InetSocketAddress(nodeRouterInterface2.GetAddress(1),sinkport));
    PacketSinkHelper packetsinkhelper("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (),sinkport));
    ApplicationContainer sinkapp = packetsinkhelper.Install(n3);
    sinkapp.Start(Seconds(0.));
    sinkapp.Stop(Seconds(10.));

    //Create TCP socket on n2
    Ptr<Socket>socket = Socket::CreateSocket(n2,TcpSocketFactory::GetTypeId ());

    //Install TCP source application on n2
    Ptr<App> app = CreateObject<App>();
    app->Setup(socket,sinkaddress,packets[j],num_packets,DataRate("100Mbps"));
    n2->AddApplication(app);
    app->SetStartTime(Seconds(1.0));
    app->SetStopTime(Seconds(10.));

    FlowMonitorHelper flowmonitor;
    Ptr<FlowMonitor> monitor = flowmonitor.InstallAll();

    //Simulation
    NS_LOG_INFO("Run Simulation");
    Simulator::Stop (Seconds(15.0));
    Simulator::Run ();

    //FairnessIndex and Throughtput Calculation
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonitor.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    double Sumx = 0, SumSqx = 0;
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

      std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ") Source Port :" << t.sourcePort << " Destination Port :" << t.destinationPort << "\n";
      std::cout << "  Tx Bytes\t\t:" << i->second.txBytes << " bytes \n";
      std::cout << "  Rx Bytes\t\t:" << i->second.rxBytes << " bytes\n";
      double time = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
      std::cout << "  Transmission Time\t:" << time << "s\n";
      double throughput = ((i->second.rxBytes * 8.0) / time)/1024;
      std::cout << "  Throughput\t\t:" << throughput  << " Kbps\n\n";

      Sumx += throughput;
      SumSqx += throughput * throughput ;
    }
    double FairnessIndex = (Sumx * Sumx)/ (2 * SumSqx) ;
    std :: cout << "Average Throughput: " << Sumx/2 << " Kbps" << std::endl;
    std :: cout << "FairnessIndex:	" << FairnessIndex << std :: endl << std::endl;
    fairnessDataset.Add (packets[j], FairnessIndex);
    std :: cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl<< std::endl;
    throughputDataset.Add(packets[j],Sumx/2);
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
  }

  //........Fairness Plot .........//
  // Add the dataset to the plot.
	plot1.AddDataset (fairnessDataset);
	// Open the plot file.
	std :: ofstream plotFile1 (fairnessPlot.c_str());
	// Write the plot file.
	plot1.GenerateOutput (plotFile1);
	// Close the plot file.
  plotFile1.close ();

  //.........Throughput Plot.........//
  // Add the dataset to the plot.
  plot2.AddDataset (throughputDataset);
  // Open the plot file.
  std :: ofstream plotFile2 (throughputPlot.c_str());
  // Write the plot file.
  plot2.GenerateOutput (plotFile2);
  // Close the plot file.
  plotFile2.close ();
}
