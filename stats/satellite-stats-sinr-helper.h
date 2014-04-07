/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
 *
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
 *
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#ifndef SATELLITE_STATS_SINR_HELPER_H
#define SATELLITE_STATS_SINR_HELPER_H

#include <ns3/satellite-stats-helper.h>
#include <ns3/ptr.h>
#include <ns3/address.h>
#include <ns3/collector-map.h>
#include <list>
#include <map>


namespace ns3 {


// BASE CLASS /////////////////////////////////////////////////////////////////

class SatHelper;
class Node;
class Time;
class DataCollectionObject;

/**
 * \ingroup satstats
 * \brief
 */
class SatStatsSinrHelper : public SatStatsHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsSinrHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsSinrHelper ();

  // inherited from ObjectBase base class
  static TypeId GetTypeId ();

  /**
   * \param minValue the MinValue attribute of the histogram, PDF, CDF output.
   */
  void SetMinValue (double minValue);

  /**
   * \return the MinValue attribute of the histogram, PDF, CDF output.
   */
  double GetMinValue () const;

  /**
   * \param maxValue the MaxValue attribute of the histogram, PDF, CDF output.
   */
  void SetMaxValue (double maxValue);

  /**
   * \return the MaxValue attribute of the histogram, PDF, CDF output.
   */
  double GetMaxValue () const;

  /**
   * \param binLength the BinLength attribute of the histogram, PDF, CDF output.
   */
  void SetBinLength (double binLength);

  /**
   * \return the BinLength attribute of the histogram, PDF, CDF output.
   */
  double GetBinLength () const;

  /**
   * \brief Set up several probes or other means of listeners and connect them
   *        to the collectors.
   */
  void InstallProbes ();

protected:
  // inherited from SatStatsHelper base class
  void DoInstall ();

  /**
   * \brief
   */
  virtual void DoInstallProbes () = 0;

  /// Maintains a list of collectors created by this helper.
  CollectorMap m_terminalCollectors;

  /// The aggregator created by this helper.
  Ptr<DataCollectionObject> m_aggregator;

private:
  double m_minValue;   ///< `MinValue` attribute.
  double m_maxValue;   ///< `MaxValue` attribute.
  double m_binLength;  ///< `BinLength` attribute.

}; // end of class SatStatsSinrHelper


// FORWARD LINK ///////////////////////////////////////////////////////////////

class Probe;

/**
 * \ingroup satstats
 * \brief Produce forward link SINR statistics from a satellite module
 *        simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsFwdPhyDelayHelper> s = Create<SatStatsFwdPhyDelayHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsFwdSinrHelper : public SatStatsSinrHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsFwdSinrHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsFwdSinrHelper ();

  // inherited from ObjectBase base class
  static TypeId GetTypeId ();

protected:
  // inherited from SatStatsDelayHelper base class
  void DoInstallProbes ();

private:
  /// Maintains a list of probes created by this helper.
  std::list<Ptr<Probe> > m_probes;

};


// RETURN LINK ////////////////////////////////////////////////////////////////

/**
 * \ingroup satstats
 * \brief Produce return link SINR statistics from a satellite module
 *        simulation.
 *
 * For a more convenient usage in simulation script, it is recommended to use
 * the corresponding methods in SatStatsHelperContainer class.
 *
 * Otherwise, the following example can be used:
 * \code
 * Ptr<SatStatsRtnSinrHelper> s = Create<SatStatsRtnSinrHelper> (satHelper);
 * s->SetName ("name");
 * s->SetIdentifierType (SatStatsHelper::IDENTIFIER_GLOBAL);
 * s->SetOutputType (SatStatsHelper::OUTPUT_SCATTER_FILE);
 * s->Install ();
 * \endcode
 */
class SatStatsRtnSinrHelper : public SatStatsSinrHelper
{
public:
  // inherited from SatStatsHelper base class
  SatStatsRtnSinrHelper (Ptr<const SatHelper> satHelper);

  /// Destructor.
  virtual ~SatStatsRtnSinrHelper ();

  // inherited from ObjectBase base class
  static TypeId GetTypeId ();

  /**
   * \brief Receive inputs from trace sources and determine the right collector
   *        to forward the inputs to.
   * \param sinrDb SINR value in dB.
   * \param from the address of the sender of the packet.
   */
  void SinrCallback (double sinrDb, const Address &from);

protected:
  // inherited from SatStatsDelayHelper base class
  void DoInstallProbes ();

private:
  /**
   * \brief Save the address and the proper identifier from the given UT node.
   * \param utNode a UT node.
   *
   * The address of the given node will be saved in the #m_identifierMap
   * member variable.
   *
   * Used in return link statistics. DoInstallProbes() is expected to pass the
   * the UT node of interest into this method.
   */
  void SaveAddressAndIdentifier (Ptr<Node> utNode);

  /// Map of address and the identifier associated with it (for return link).
  std::map<const Address, uint32_t> m_identifierMap;

}; // end of class SatStatsRtnSinrHelper


} // end of namespace ns3


#endif /* SATELLITE_STATS_SINR_HELPER_H */