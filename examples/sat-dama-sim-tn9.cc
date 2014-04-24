/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/cbr-helper.h"
#include "ns3/config-store.h"


using namespace ns3;

#define CALL_SAT_STATS_BASIC_SET(id)                                          \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                    \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                   \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                   \
                                                                              \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                     \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                    \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                    \
                                                                              \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                   \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                  \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                  \
                                                                              \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                     \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                    \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                    \

#define CALL_SAT_STATS_DISTRIBUTION_SET(id)                                   \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                    \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                   \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);                 \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_PDF_FILE);                       \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_CDF_FILE);                       \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                   \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);                 \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_PDF_PLOT);                       \
  s->AddGlobal ## id (SatStatsHelper::OUTPUT_CDF_PLOT);                       \
                                                                              \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                     \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                    \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);                  \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_PDF_FILE);                        \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_CDF_FILE);                        \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                    \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);                  \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_PDF_PLOT);                        \
  s->AddPerGw ## id (SatStatsHelper::OUTPUT_CDF_PLOT);                        \
                                                                              \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                   \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                  \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);                \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_PDF_FILE);                      \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_CDF_FILE);                      \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                  \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);                \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_PDF_PLOT);                      \
  s->AddPerBeam ## id (SatStatsHelper::OUTPUT_CDF_PLOT);                      \
                                                                              \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCALAR_FILE);                     \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCATTER_FILE);                    \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);                  \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_PDF_FILE);                        \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_CDF_FILE);                        \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_SCATTER_PLOT);                    \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);                  \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_PDF_PLOT);                        \
  s->AddPerUt ## id (SatStatsHelper::OUTPUT_CDF_PLOT);

/**
* \ingroup satellite
*
* \brief Simulation script to run example simulation results related to satellite RTN
* link performance. Currently only one beam is simulated with variable amount of users
* and DAMA configuration.
*
* execute command -> ./waf --run "sat-dama-sim-tn9 --PrintHelp"
*/

NS_LOG_COMPONENT_DEFINE ("sat-dama-sim-tn9");

int
main (int argc, char *argv[])
{
  uint32_t beamId = 1;
  uint32_t endUsersPerUt (1);
  uint32_t utsPerBeam (3);
  uint32_t damaConf (0);

  Time simLength (Seconds(50.0));
  Time appStartTime = Seconds(0.1);

  // CBR parameters
  uint32_t minPacketSizeBytes (800); // -> 128 kbps
  uint32_t maxPacketSizeBytes (6400); // -> 1024 kbps
  Time interval (MilliSeconds(50));

  // To read attributes from file
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("./src/satellite/examples/tn9-dama-input-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  Ptr<UniformRandomVariable> randVariable = CreateObject<UniformRandomVariable> ();

  /**
   * Attributes:
   * -----------
   *
   * Scenario: 1 beam (beam id = 1)
   *
   * Frame configuration:
   * - 3 frames
   * - 125 MHz user bandwidth
   *    - 40 x 0.625 MHz -> 25 MHz
   *    - 40 x 1.25 MHz -> 50 MHz
   *    - 20 x 2.5 MHz -> 50 MHz
   *
   * Conf-2 scheduling mode (dynamic time slots)
   * - FCA disabled
   *
   * RTN link
   *   - ACM enabled
   *   - Constant interference
   *   - AVI error model
   *   - ARQ disabled
   * FWD link
   *   - ACM disabled
   *   - Constant interference
   *   - No error model
   *   - ARQ disabled
   *
   */

  // read command line parameters given by user
  CommandLine cmd;
  cmd.AddValue("utsPerBeam", "Number of UTs per spot-beam", utsPerBeam);
  cmd.AddValue("damaConf", "DAMA configuration", damaConf);
  cmd.Parse (argc, argv);

  switch (damaConf)
  {
    // CRA only
    case 0:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=20]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=128]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue(false));
        break;
      }
    // CRA + RBDC
    case 1:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=20]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue(false));
        break;
      }
    // CRA + VBDC
    case 2:
      {
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantAssignmentProvided", BooleanValue(true));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_ConstantServiceRate", StringValue ("ns3::ConstantRandomVariable[Constant=20]"));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService0_VolumeAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_ConstantAssignmentProvided", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_RbdcAllowed", BooleanValue(false));
        Config::SetDefault ("ns3::SatLowerLayerServiceConf::DaService1_VolumeAllowed", BooleanValue(true));
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported damaConf: " << damaConf);
        break;
      }
  }

  std::string scenarioName = "Scenario72";

  Ptr<SatHelper> helper = CreateObject<SatHelper> (scenarioName);

  // create user defined scenario
  SatBeamUserInfo beamInfo = SatBeamUserInfo (utsPerBeam, endUsersPerUt);
  std::map<uint32_t, SatBeamUserInfo > beamMap;
  beamMap[beamId] = beamInfo;
  helper->SetBeamUserInfo (beamMap);
  helper->EnablePacketTrace ();

  helper->CreateScenario (SatHelper::USER_DEFINED);

  // enable info logs
  //LogComponentEnable ("CbrApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  //LogComponentEnable ("sat-cbr-user-defined-example", LOG_LEVEL_INFO);

  // get users
  NodeContainer utUsers = helper->GetUtUsers();
  NodeContainer gwUsers = helper->GetGwUsers();

  // >>> Start of actual test using Full scenario >>>

  // port used for packet delivering
  uint16_t port = 9; // Discard port (RFC 863)
  const std::string protocol = "ns3::UdpSocketFactory";

  /**
   * Set-up CBR traffic
   */
  const InetSocketAddress gwAddr = InetSocketAddress (helper->GetUserAddress (gwUsers.Get (0)), port);

  for (NodeContainer::Iterator itUt = utUsers.Begin ();
      itUt != utUsers.End ();
      ++itUt)
    {
      appStartTime += MilliSeconds (10);

      // return link
      Ptr<CbrApplication> rtnApp = CreateObject<CbrApplication> ();
      rtnApp->SetAttribute ("Protocol", StringValue (protocol));
      rtnApp->SetAttribute ("Remote", AddressValue (gwAddr));
      rtnApp->SetAttribute ("Interval", TimeValue (interval));

      // Random static packet size
      uint32_t size = randVariable->GetInteger (minPacketSizeBytes, maxPacketSizeBytes);
      rtnApp->SetAttribute ("PacketSize", UintegerValue (size));

      rtnApp->SetStartTime (appStartTime);
      rtnApp->SetStopTime (simLength);
      (*itUt)->AddApplication (rtnApp);
    }

  // setup packet sinks at all users
  Ptr<PacketSink> ps = CreateObject<PacketSink> ();
  ps->SetAttribute ("Protocol", StringValue (protocol));
  ps->SetAttribute ("Local", AddressValue (gwAddr));
  gwUsers.Get (0)->AddApplication (ps);

  /**
   * Set-up statistics
   */

  Ptr<SatStatsHelperContainer> s = CreateObject<SatStatsHelperContainer> (helper);
  s->SetName ("cbr");

  /*
   * The following is the statements for enabling some satellite statistics
   * for testing purpose.
   */
  s->AddPerUtRtnAppDelay (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
  s->AddPerGwRtnDevDelay (SatStatsHelper::OUTPUT_CDF_PLOT);
  s->AddPerUtRtnMacDelay (SatStatsHelper::OUTPUT_HISTOGRAM_FILE);
  s->AddPerGwRtnPhyDelay (SatStatsHelper::OUTPUT_CDF_FILE);

  s->AddPerUtUserRtnAppThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);
  s->AddPerBeamRtnDevThroughput (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnMacThroughput (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerBeamRtnPhyThroughput (SatStatsHelper::OUTPUT_SCALAR_FILE);

  s->AddGlobalRtnQueuePackets (SatStatsHelper::OUTPUT_PDF_FILE);
  s->AddPerBeamRtnSinr (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddGlobalRtnSignallingLoad (SatStatsHelper::OUTPUT_SCATTER_PLOT);
  s->AddPerUtCapacityRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);
  s->AddPerBeamResourcesGranted (SatStatsHelper::OUTPUT_HISTOGRAM_PLOT);
  s->AddPerGwBackloggedRequest (SatStatsHelper::OUTPUT_SCATTER_FILE);

  NS_LOG_INFO("--- Cbr-user-defined-example ---");
  NS_LOG_INFO("  Packet sending interval: " << interval.GetSeconds ());
  NS_LOG_INFO("  Simulation length: " << simLength.GetSeconds ());
  NS_LOG_INFO("  Number of UTs: " << utsPerBeam);
  NS_LOG_INFO("  Number of end users per UT: " << endUsersPerUt);
  NS_LOG_INFO("  ");

  /**
   * Store attributes into XML output
   */
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("tn9-dama-output-attributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("Xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();
  outputConfig.ConfigureAttributes ();

  /**
   * Run simulation
   */
  Simulator::Stop (simLength);
  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
