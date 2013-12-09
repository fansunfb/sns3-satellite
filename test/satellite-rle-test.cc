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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

/**
 * \file satellite-rle-test.cc
 * \brief Test cases to unit test RLE
 */

#include <vector>
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/random-variable-stream.h"
#include "../model/satellite-return-link-encapsulator.h"

using namespace ns3;

/**
 * \brief Test case to unit test RLE
 *
 */
class SatRleTestCase : public TestCase
{
public:
  SatRleTestCase ();
  virtual ~SatRleTestCase ();

  /**
   * Receive packet and check that it is of correct size
   */
  void Receive (Ptr<Packet> p);

private:
  virtual void DoRun (void);

  /**
   * Sent packet sizes
   */
  std::vector<uint32_t> m_sentPacketSizes;

  /**
   * Received packet sizes
   */
  std::vector<uint32_t> m_rcvdPacketSizes;
};

SatRleTestCase::SatRleTestCase ()
  : TestCase ("Test RLE.")
{
}

SatRleTestCase::~SatRleTestCase ()
{
}


void
SatRleTestCase::DoRun (void)
{
  Mac48Address source = Mac48Address::Allocate();
  Mac48Address dest = Mac48Address::Allocate();

  Ptr<SatReturnLinkEncapsulator> rle = CreateObject<SatReturnLinkEncapsulator> (source, dest);

  // Create a receive callback to Receive method of this class.
  rle->SetReceiveCallback (MakeCallback (&SatRleTestCase::Receive, this));

  // Random variable for sent packet sizes and tx opportunities
  Ptr<UniformRandomVariable> unif = CreateObject<UniformRandomVariable> ();

  // Number of created packets
  uint32_t numPackets (100);

  // Create packets and push them to RLE
  for (uint32_t i = 0; i < numPackets; ++i)
    {
      uint32_t packetSize = unif->GetInteger (3, 1500);
      Ptr<Packet> packet = Create<Packet> (packetSize);
      m_sentPacketSizes.push_back (packetSize);
      rle->TransmitPdu (packet);
    }

  /**
   * Create TxOpportunities for RLE and call receive method to do decapsuling,
   * defragmentation and reassembly.
   */
  uint32_t bytesLeft (1);
  while (bytesLeft > 0)
    {
      Ptr<Packet> p = rle->NotifyTxOpportunity (unif->GetInteger (3, 1500), bytesLeft);
      rle->ReceivePdu (p);
    }

  /**
   * Test that the amount of sent packets is the same as the amount of received packets.
   */
  NS_TEST_ASSERT_MSG_EQ( m_sentPacketSizes.size(), m_rcvdPacketSizes.size(), "All sent packets are not received");
}

void SatRleTestCase::Receive (Ptr<Packet> p)
{
  uint32_t rcvdPacketSize = p->GetSize ();
  m_rcvdPacketSizes.push_back (rcvdPacketSize);
  uint32_t numRcvdPackets = m_rcvdPacketSizes.size ();

  /**
   * Test the sent packet size is the same as the received packet size. This tests the
   * encapsulation, fragmentation and packing functionality as well as reassembly.
   */
  NS_TEST_ASSERT_MSG_EQ( m_sentPacketSizes[numRcvdPackets-1], m_rcvdPacketSizes[numRcvdPackets-1], "Wrong size packet received");
}

/**
 * \brief Test suite for RLE.
 */
class SatRleTraceSuite : public TestSuite
{
public:
  SatRleTraceSuite ();
};

SatRleTraceSuite::SatRleTraceSuite ()
  : TestSuite ("sat-rle-test", UNIT)
{
  AddTestCase (new SatRleTestCase);
}

// Do allocate an instance of this TestSuite
static SatRleTraceSuite SatRleTestSuite;
