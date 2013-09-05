/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/pointer.h"

#include "satellite-phy.h"
#include "satellite-phy-rx.h"
#include "satellite-phy-tx.h"
#include "satellite-channel.h"
#include "satellite-mac.h"
#include "satellite-signal-parameters.h"


NS_LOG_COMPONENT_DEFINE ("SatPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPhy);

SatPhy::SatPhy (void)
{
  NS_LOG_FUNCTION (this);
}

SatPhy::SatPhy (Ptr<SatPhyTx> phyTx, Ptr<SatPhyRx> phyRx, uint32_t beamId, SatPhy::ReceiveCallback cb)
  :
  m_phyTx(phyTx),
  m_phyRx(phyRx),
  m_beamId(beamId),
  m_rxCallback(cb)
{
  NS_LOG_FUNCTION (this << phyTx << phyRx << beamId);

  ObjectBase::ConstructSelf(AttributeConstructionList ());

  // calculate EIRP without Gain (maximum)
  double eirpWoGain_Db = m_maxPower_Db - m_outputLoss_Db - m_pointingLoss_Db - m_oboLoss_Db - m_antennaLoss_Db;

  // TODO: needed to have 'utils'  library to make these kind of conversions
  m_eirpWoGain_W = std::pow( 10.0, eirpWoGain_Db / 10.0 );

  phyTx->SetBeamId(beamId);
  phyRx->SetBeamId(beamId);

  phyRx->SetReceiveCallback( MakeCallback (&SatPhy::Receive, this) );
}


SatPhy::~SatPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
SatPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_phyTx->DoDispose();
  m_phyTx = 0;
  m_phyRx->DoDispose();
  m_phyRx = 0;

  Object::DoDispose ();
}

TypeId
SatPhy::GetInstanceTypeId (void) const
{
  return GetTypeId();
}

TypeId
SatPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPhy")
    .SetParent<Object> ()
    .AddConstructor<SatPhy> ()
    .AddAttribute ("PhyRx", "The PhyRx layer attached to this phy.",
                   PointerValue (),
                   MakePointerAccessor (&SatPhy::GetPhyRx,
                                        &SatPhy::SetPhyRx),
                   MakePointerChecker<SatPhyRx> ())
    .AddAttribute("MaxGainDb", "Maximum RX gain in Dbs",
                   DoubleValue(0.00),
                   MakeDoubleAccessor(&SatPhy::m_maxGain_Db),
                   MakeDoubleChecker<double> ())
    .AddAttribute("MaxPowerDb", "Maximum TX power in Dbs",
                   DoubleValue(0.00),
                   MakeDoubleAccessor(&SatPhy::m_maxPower_Db),
                   MakeDoubleChecker<double> ())
    .AddAttribute("OutputLossDb", "TX Output loss in Dbs",
                   DoubleValue(0.00),
                   MakeDoubleAccessor(&SatPhy::m_outputLoss_Db),
                   MakeDoubleChecker<double> ())
    .AddAttribute("PointingLossDb", "TX Pointing loss in Dbs",
                   DoubleValue(0.00),
                   MakeDoubleAccessor(&SatPhy::m_pointingLoss_Db),
                   MakeDoubleChecker<double> ())
    .AddAttribute("OboLossDb", "TX OBO loss in Dbs",
                   DoubleValue(0.00),
                   MakeDoubleAccessor(&SatPhy::m_oboLoss_Db),
                   MakeDoubleChecker<double> ())
    .AddAttribute("AntennaLossDb", "TX Antenna loss in Dbs",
                   DoubleValue(0.00),
                   MakeDoubleAccessor(&SatPhy::m_antennaLoss_Db),
                   MakeDoubleChecker<double> ())

  ;
  return tid;
}

void
SatPhy::DoStart ()
{
  NS_LOG_FUNCTION (this);
  Object::DoStart ();
}

Ptr<SatPhyTx>
SatPhy::GetPhyTx () const
{
  NS_LOG_FUNCTION (this);
  return m_phyTx;
}

Ptr<SatPhyRx>
SatPhy::GetPhyRx () const
{
  NS_LOG_FUNCTION (this);
  return m_phyRx;
}

void
SatPhy::SetPhyTx (Ptr<SatPhyTx> phyTx)
{
  NS_LOG_FUNCTION (this << phyTx);
  m_phyTx = phyTx;
}

void
SatPhy::SetPhyRx (Ptr<SatPhyRx> phyRx)
{
  NS_LOG_FUNCTION (this << phyRx);
  m_phyRx = phyRx;
}

Ptr<SatChannel>
SatPhy::GetTxChannel ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phyTx);

  return m_phyTx->GetChannel ();
}

double
SatPhy::GetTxPower_W () const
{
  NS_LOG_FUNCTION (this);
  return m_eirpWoGain_W;
}

void
SatPhy::SendPdu (Ptr<Packet> p, uint32_t carrierId, Time duration )
{
  NS_LOG_FUNCTION (this << p << carrierId << duration);
  NS_LOG_LOGIC (this << " sending a packet with carrierId: " << carrierId << " duration: " << duration);

  // Create a new SatSignalParameters related to this packet transmission
  Ptr<SatSignalParameters> txParams = Create<SatSignalParameters> ();
  txParams->m_duration = duration;
  txParams->m_phyTx = m_phyTx;
  txParams->m_packet = p;
  txParams->m_beamId = m_beamId;
  txParams->m_carrierId = carrierId;

  // TODO: frequency initialized according to carrier config
  double FREQUENCY (17.9 * std::pow(10.0, 9));
  txParams->m_frequency_Hz = FREQUENCY;
  txParams->m_txPower_W = m_eirpWoGain_W;

  m_phyTx->StartTx (p, txParams);
}

void
SatPhy::SendPdu (Ptr<Packet> p, Ptr<SatSignalParameters> txParams )
{
  NS_LOG_FUNCTION (this << p << txParams);
  NS_LOG_LOGIC (this << " sending a packet with carrierId: " << txParams->m_carrierId << " duration: " << txParams->m_duration);

  m_phyTx->StartTx (p, txParams);
}

void
SatPhy::SetBeamId (uint32_t beamId)
{
  NS_LOG_FUNCTION (this << beamId);
  m_beamId = beamId;
  m_phyTx->SetBeamId (beamId);
  m_phyRx->SetBeamId (beamId);
}

void
SatPhy::Receive (Ptr<SatSignalParameters> rxParams)
{
  NS_LOG_FUNCTION (this << rxParams);
  m_rxCallback( rxParams->m_packet, rxParams);
}


} // namespace ns3
