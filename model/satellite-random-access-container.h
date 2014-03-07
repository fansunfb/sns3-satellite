/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd.
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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */
#ifndef SATELLITE_RANDOM_ACCESS_H
#define SATELLITE_RANDOM_ACCESS_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "satellite-random-access-container-conf.h"
#include "ns3/random-variable-stream.h"
#include <set>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Class for random access algorithms. The purpose of this class is to implement
 * Slotted ALOHA and CRDSA algorithms for randomizing the Tx opportunities. In Slotted
 * ALOHAs case this means randomizing the wait time after which the next slot is selected
 * for transmission. In CRDSAs case this means randomizing the slots within a frame for
 * each unique packet and it's replicas.
 */
class SatRandomAccess : public Object
{
public:

  /**
   * \brief Typedef of callback for known DAMA status
   * \return Is there known DAMA allocations
   */
  typedef Callback<bool> IsDamaAvailableCallback;

  /**
   * \brief Typedef of callback for buffer status
   * \return Is there data left in the buffers
   */
  typedef Callback<bool> AreBuffersEmptyCallback;

  /**
   * \brief Typedef of callback for packets matching the size conditions for RA
   * \return How many packets small enough for RA is in the buffers
   */
  typedef Callback<uint32_t,uint32_t> NumOfCandidatePacketsCallback;

  /**
   * \enum RandomAccessModel_t
   * \brief The defined random access models. These define the implemented algorithms
   */
  typedef enum
  {
    RA_OFF = 0,
    RA_SLOTTED_ALOHA = 1,
    RA_CRDSA = 2,
    RA_ANY_AVAILABLE = 3
  } RandomAccessModel_t;

  /**
   * \enum RandomAccessTriggerType_t
   * \brief The defined random access trigger types. These help determine
   * which algorithm to use if multiple algorithms are enabled
   */
  typedef enum
  {
    RA_SLOTTED_ALOHA_TRIGGER = 0,
    RA_CRDSA_TRIGGER = 1
  } RandomAccessTriggerType_t;

  /**
   * \enum RandomAccessTxOpportunityType_t
   * \brief Random access Tx opportunity types. These are used to define to which algorithm
   * the results provided by this module applies to
   */
  typedef enum
  {
    RA_DO_NOTHING = 0,
    RA_SLOTTED_ALOHA_TX_OPPORTUNITY = 1,
    RA_CRDSA_TX_OPPORTUNITY = 2,
  } RandomAccessTxOpportunityType_t;

  /**
   * \struct RandomAccessTxOpportunities_s
   * \brief Random access Tx opportunities. This struct contains the algorithm results from this module
   */
  typedef struct
  {
    RandomAccessTxOpportunityType_t txOpportunityType;
    uint32_t slottedAlohaTxOpportunity;
    std::vector <std::set<uint32_t> > crdsaTxOpportunities;
    uint32_t allocationChannel;
  } RandomAccessTxOpportunities_s;

  /**
   * \brief Constructor
   */
  SatRandomAccess ();

  /**
   * \brief Constructor
   */
  SatRandomAccess (Ptr<SatRandomAccessConf> randomAccessConf, RandomAccessModel_t randomAccessModel);

  /**
   * \brief Destructor
   */
  virtual ~SatRandomAccess ();

  /**
   * \brief NS-3 type id function
   * \return type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function for setting the used random access model
   * \param randomAccessModel random access model
   */
  void SetRandomAccessModel (RandomAccessModel_t randomAccessModel);

  /**
   * \brief Function for setting the load control parameters
   * \param allocationChannel allocation channel
   * \param backoffProbability backoff probability
   * \param backoffTime backoff time
   */
  void CrdsaSetLoadControlParameters (uint32_t allocationChannel,
                                      double backoffProbability,
                                      uint32_t backoffTime);

  /**
   * \brief Function for setting the maximum backoff probability
   * \param allocationChannel allocation channel
   * \param maximumBackoffProbability maximum backoff probability
   */
  void CrdsaSetMaximumBackoffProbability (uint32_t allocationChannel,
                                          double maximumBackoffProbability);

  /**
   * \brief Function for setting the maximum CRDSA payload bytes
   * \param allocationChannel allocation channel
   * \param payloadBytes payload bytes
   */
  void CrdsaSetPayloadBytes (uint32_t allocationChannel,
                             uint32_t payloadBytes);
  /**
   * \brief Function for setting the parameters related to CRDSA randomization
   * \param allocationChannel allocation channel
   * \param minRandomizationValue minimum randomization value
   * \param maxRandomizationValue maximum randomization value
   * \param numOfInstances number of packet instances (packet replicas)
   */
  void CrdsaSetRandomizationParameters (uint32_t allocationChannel,
                                        uint32_t minRandomizationValue,
                                        uint32_t maxRandomizationValue,
                                        uint32_t numOfInstances);

  /**
   * \brief Function for setting the maximum rate limitation parameters
   * \param allocationChannel allocation channel
   * \param maxUniquePayloadPerBlock maximum number of unique payloads per block
   * \param maxConsecutiveBlocksAccessed maximum number of consecutive blocks accessed
   * \param minIdleBlocks minimum number of idle blocks
   */
  void CrdsaSetMaximumDataRateLimitationParameters (uint32_t allocationChannel,
                                                    uint32_t maxUniquePayloadPerBlock,
                                                    uint32_t maxConsecutiveBlocksAccessed,
                                                    uint32_t minIdleBlocks);

  /**
   * \brief Function for setting the Slotted ALOHA control randomization interval
   * \param controlRandomizationInterval control randomization interval
   */
  void SlottedAlohaSetControlRandomizationInterval (double controlRandomizationInterval);

  /**
   * \brief Main function of this module. This will be called from outside and it is responsible
   * for selecting the appropriate RA algorithm
   * \param allocationChannel allocation channel
   * \param triggerType RA trigger type
   * \return RA algorithm results (Tx opportunities)
   */
  SatRandomAccess::RandomAccessTxOpportunities_s DoRandomAccess (uint32_t allocationChannel, RandomAccessTriggerType_t triggerType);

  /**
   * \brief Function for setting the IsDamaAvailable callback
   * \param callback callback
   */
  void SetIsDamaAvailableCallback (SatRandomAccess::IsDamaAvailableCallback callback);

  /**
   * \brief Function for setting the AreBuffersEmpty callback
   * \param callback callback
   */
  void SetAreBuffersEmptyCallback (SatRandomAccess::AreBuffersEmptyCallback callback);

  /**
   * \brief Function for setting the NumOfCandidatePackets callback
   * \param callback callback
   */
  void SetNumOfCandidatePacketsCallback (SatRandomAccess::NumOfCandidatePacketsCallback callback);

protected:

  /**
   * \brief Function for disposing the module and its variables
   */
  void DoDispose ();

private:

  /**
   * \brief Function for checking whether the CRDSA backoff probability is higher than the parameterized value.
   * This affects algorithm selection in the case both CRDSA and Slotted ALOHA are enabled
   * \param allocationChannel allocation channel
   * \return Is CRDSA backoff probability too high
   */
  bool IsCrdsaBackoffProbabilityTooHigh (uint32_t allocationChannel);

  /**
   * \brief Function for printing out various module variables to console
   */
  void PrintVariables ();

  /**
   * \brief Main function for Slotted ALOHA
   * \return Slotted ALOHA algorithm results (Tx opportunity)
   */
  SatRandomAccess::RandomAccessTxOpportunities_s DoSlottedAloha ();

  /**
    * \brief Function for performing the Slotted ALOHA release time randomization, i.e., the time
    * after which the next available slot is selected for Tx opportunity. Control randomization
    * interval defines the maximum release time.
    * \return randomized time
    */
  uint32_t SlottedAlohaRandomizeReleaseTime ();

  /**
   * \brief Function for checking the sanity of Slotted ALOHA related variables
   */
  void SlottedAlohaDoVariableSanityCheck ();

  /**
   * \brief Main function for CRDSA algorithm
   * \param allocationChannel allocation channel
   * \return CRDSA algorithm results (Tx opportunities)
   */
  SatRandomAccess::RandomAccessTxOpportunities_s DoCrdsa (uint32_t allocationChannel);

  /**
   * \brief Function for checking whether the backoff time has passed for this allocation channel
   * \param allocationChannel allocation channel
   * \return Has backoff time passed
   */
  bool CrdsaHasBackoffTimePassed (uint32_t allocationChannel);

  /**
   * \brief Function for evaluating the backoff for this allocation channel
   * \param allocationChannel allocation channel
   * \return Was backoff performed or not
   */
  bool CrdsaDoBackoff (uint32_t allocationChannel);

  /**
   * \brief Function for randomizing the CRDSA Tx opportunities (slots) for each unique packet
   * \param allocationChannel allocation channel
   * \param slots a set of reserved slots
   * \return a set of reserved slots updated with the results of the latest randomization
   */
  std::pair <std::set<uint32_t>, std::set<uint32_t> > CrdsaRandomizeTxOpportunities (uint32_t allocationChannel, std::pair <std::set<uint32_t>, std::set<uint32_t> > slots);

  /**
   * \brief Function for evaluating backoff for each unique CRDSA packet and calling the
   * randomization of Tx opportunities if backoff is not triggered
   * \param allocationChannel allocation channel
   * \return Tx opportunities for all unique packets
   */
  SatRandomAccess::RandomAccessTxOpportunities_s CrdsaPrepareToTransmit (uint32_t allocationChannel);

  /**
   * \brief Function for setting the allocation channel specific backoff timer
   * \param allocationChannel allocation channel
   */
  void CrdsaSetBackoffTimer (uint32_t allocationChannel);

  /**
   * \brief Function for increasing the allocation channel specific count of consecutive used blocks.
   * If the maximum number of consecutive blocks is reached, this function triggers the call of idle
   * period.
   * \param allocationChannel allocation channel
   */
  void CrdsaIncreaseConsecutiveBlocksUsed (uint32_t allocationChannel);

  /**
   * \brief Function for reducing the allocation channel specific number of idle blocks in effect
   * \param allocationChannel allocation channel
   */
  void CrdsaReduceIdleBlocks (uint32_t allocationChannel);

  /**
   * \brief Function for reducing the idle blocks in effect for all allocation channels
   */
  void CrdsaReduceIdleBlocksForAllAllocationChannels ();

  /**
   * \brief Function for checking if the allocation channel is free
   * \param allocationChannel allocation channel
   * \return Is the allocation channel free
   */
  bool CrdsaIsAllocationChannelFree (uint32_t allocationChannel);

  /**
   * \brief Uniform random variable object
   */
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  /**
   * \brief The used random access model
   */
  RandomAccessModel_t m_randomAccessModel;

  /**
   * \brief A pointer to random access configuration
   */
  Ptr<SatRandomAccessConf> m_randomAccessConf;

  /**
   * \brief Number of allocation channels available
   */
  uint32_t m_numOfAllocationChannels;

  /**
   * \brief A flag defining whether the buffers were emptied the last time RA was
   * evaluated, i.e., is the data now new
   */
  bool m_crdsaNewData;

  /**
    * \brief Callback for known DAMA status
   */
  IsDamaAvailableCallback m_isDamaAvailableCb;

  /**
    * \brief Callback for buffer status
   */
  AreBuffersEmptyCallback m_areBuffersEmptyCb;

  /**
    * \brief Callback for packets matching the size conditions for RA
   */
  NumOfCandidatePacketsCallback m_numOfCandidatePacketsCb;
};

} // namespace ns3

#endif /* SATELLITE_RANDOM_ACCESS_H */
