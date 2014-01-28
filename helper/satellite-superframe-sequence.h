/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#ifndef SATELLITE_SUPERFRAME_SEQUENCE_H
#define SATELLITE_SUPERFRAME_SEQUENCE_H

#include <vector>
#include <map>
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/satellite-control-message.h"
#include "satellite-frame-conf.h"
#include "satellite-wave-form-conf.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements super frame sequence.
 *  It consists information of the super frames in sequence.
 *
 */
class SatSuperframeSeq : public Object
{
public:
  typedef std::vector<Ptr<SatSuperframeConf> > SatSuperframeConfList;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor for SatSuperframeConf
   */
  SatSuperframeSeq ();

  /**
   * Constructor for SatSuperframeSeq.
   *
   * \param confs Superframes in sequence. (In acsending order according to frequency).
   */
  SatSuperframeSeq ( SatSuperframeConfList * confs );

  /**
   * Destructor for SatSuperframeSeq.
   */
  ~SatSuperframeSeq ();

  /**
   * Add waveform configuration class instance to this superframe sequence
   * \param wfConf Waveform conf to add
   */
  void AddWaveformConf (Ptr<SatWaveformConf> wfConf);

  /**
   * Get waveform configuration
   * \return SatWaveformConf Waveform configuration class instance
   */
  Ptr<SatWaveformConf> GetWaveformConf () const;

  /**
   * Add super frame (configuration) to super frame sequence
   * \param conf Super frame configuration to add. (id of the sequence is order number of addition starting from 0)
   */
  void AddSuperframe (Ptr<SatSuperframeConf> conf);

  /**
   * Get superframe conf of the sequence.
   *
   * \param seqId    Id of the super frame sequence requested.
   * \return The requested super frame conf of the sequence.
   */
  Ptr<SatSuperframeConf> GetSuperframeConf  (uint8_t seqId ) const;

  /**
   * Get global carrier id. Converts super frame specific id to global carrier id.
   *
   * \param superframeId       Id of the super frame requested.
   * \param frameId            Id of the frame inside super frame requested.
   * \param frameCarrierId     Id of the carrier inside frame requested.
   *
   * \return The requested carrier id of the super frame.
   */
  uint32_t GetCarrierId( uint8_t superframeId, uint8_t frameId, uint16_t frameCarrierId ) const;

  /**
   * Get carrier count of the super frame sequence.
   *
   * \return The super frame sequence carrier count.
   */
  uint32_t GetCarrierCount () const;

  /**
   * Get carrier count in the super frame.
   *
   * \param seqId Sequence id of the super frame which carrier count is requested.
   *
   * \return The super frame carrier count.
   */
  uint32_t GetCarrierCount ( uint8_t seqId ) const;

  /**
   * Get duration of the super frame.
   *
   * \param seqId Sequence id of the super frame which duration is requested.
   *
   * \return The super frame duration in seconds.
   */
  double GetDurationInSeconds ( uint8_t seqId ) const;

  /**
   * Get the center frequency of the requested carrier.
   *
   * \param carrierId  Id of the carrier inside super frame which center frequency is requested.
   *
   * \return The center frequency of the requested carrier.
   */
  double GetCarrierFrequencyHz (uint32_t carrierId) const;

  /**
   * Get the bandwidth of the requested carrier.
   *
   * \param carrierId  Id of the carrier inside super frame which bandwidth is requested.
   * \param bandwidthType Type of bandwidth requested.
   *
   * \return The bandwidth of the requested carrier.
   */
  double GetCarrierBandwidthHz (uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType) const;

  /**
   *
   * \param beamId  ID of the beam, where added TBTP is assigned.
   * \param tbtpMsg Pointer to TBTP message to add.
   * \return ID assigned to added TBTP message.
   */
  uint32_t AddTbtpMessage (uint32_t beamId, Ptr<SatTbtpMessage> tbtpMsg);

  /**
   *
   * \param beamId  ID of the beam, which TBTP is requested.
   * \param msgId   ID assigned to added TBTP message.
   * \return Pointer to TBTP message.
   */
  Ptr<SatTbtpMessage> GetTbtpMessage (uint32_t beamId, uint32_t msgId) const;

private:
  typedef std::map<uint32_t, Ptr<SatTbtpContainer> > TbtpMap_t;

  /**
    * Super frame sequences.
    *
    * Table includes super frame configurations for the return link.
    * Item index of the list means super frame sequency (SFS).
    */
  SatSuperframeConfList m_superframe;

  /**
   * Waveform configurations
   */
  Ptr<SatWaveformConf> m_wfConf;

  /**
   * Map to store TBTP containers.
   */
  TbtpMap_t tbtpContainers;

  /**
   * Time to store TBTP messages in container. (time used to calculate number of TBTP messages to store).
   */
  Time m_tbtpStoreTime;

};

} // namespace ns3

#endif // SATELLITE_SUPERFRAME_SEQUENCE_H
