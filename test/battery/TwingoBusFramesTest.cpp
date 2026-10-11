#include <gtest/gtest.h>

#include <cstdio>
#include <set>

#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BUS.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-LOGIC.h"

using namespace twingo_bus;

namespace {

// Real frames of the car log 22aaf176 (0x0EC, 3 bytes).
TEST(TwingoBus, Crc0ecMatchesFourRealFrames) {
  const uint8_t real[4][3] = {{0x10, 0x01, 0x17}, {0x10, 0x11, 0xDA}, {0x10, 0x21, 0x90}, {0x10, 0x31, 0x5D}};
  for (const auto& r : real) {
    EXPECT_EQ((uint8_t)(crc8(r, 2, 0x1D) ^ 0xBE), r[2]);
  }
}

TEST(TwingoBus, TableHas30UniqueIdsWithKnownCycleTimes) {
  std::set<uint16_t> ids;
  for (uint8_t i = 0; i < FRAME_COUNT; i++) {
    const FrameDef& d = FRAMES[i];
    ids.insert(d.id);
    EXPECT_TRUE(d.interval_ms == 10 || d.interval_ms == 100 || d.interval_ms == 1000 || d.interval_ms == 3000) << d.id;
    EXPECT_GE(d.dlc, 2);
    EXPECT_LE(d.dlc, 8);
    EXPECT_EQ(find_frame(d.id), &FRAMES[i]);
  }
  EXPECT_EQ(ids.size(), (size_t)FRAME_COUNT);
  EXPECT_EQ(find_frame(0x350), nullptr);  // vehicle CAN only
  EXPECT_EQ(find_frame(0x5A1), nullptr);  // cell frames: LED only, never sent
}

TEST(TwingoBus, CycleTimesAndLengthsOfTheFiveZoeFrames) {
  EXPECT_EQ(find_frame(0x19F)->interval_ms, 10);  // 10 ms in the car (100 ms in the old Zoe form)
  EXPECT_EQ(find_frame(0x423)->interval_ms, 100);
  EXPECT_EQ(find_frame(0x426)->interval_ms, 100);
  EXPECT_EQ(find_frame(0x436)->interval_ms, 100);
  EXPECT_EQ(find_frame(0x69F)->interval_ms, 1000);
  EXPECT_EQ(find_frame(0x436)->dlc, 6);
  EXPECT_EQ(find_frame(0x69F)->dlc, 4);
}

TEST(TwingoBus, Frame19fCounterStepsFiveModuloSixteen) {
  const FrameDef& d = *find_frame(0x19F);
  uint8_t out[8];
  // the counter starts with F in the first frame of the session and runs through the wake-up form
  for (uint32_t s = 22; s < 30; s++) {
    build_frame(d, BUS_WACH, s, 0, 0, out);
    EXPECT_EQ(out[3] & 0x0F, (0x0F + 5 * s) & 0x0F) << s;
    EXPECT_EQ(out[2], 0x7D);
    EXPECT_EQ(out[7], 0xFE);
    EXPECT_EQ(out[5], 0x6A);
  }
}

TEST(TwingoBus, Frame19fChecksumIsTheCrcOfTheTwelveBitValue) {
  EXPECT_EQ(crc19f(0x7D, 0x0), 0x6A);
  EXPECT_EQ(crc19f(0x7D, 0x1), 0x45);
  EXPECT_EQ(crc19f(0x7D, 0x2), 0x34);
  EXPECT_EQ(crc19f(0x7D, 0x3), 0x1B);
  EXPECT_EQ(crc19f(0xB7, 0x3), 0xDB);
  EXPECT_EQ(crc19f(0x7C, 0xF), 0x5E);
  EXPECT_EQ(crc19f(0xFF, 0xF), 0x06);
}

TEST(TwingoBus, Frame19fWakeFormIsTheOneOfTheCarLog) {
  const FrameDef& d = *find_frame(0x19F);
  const char* want[24] = {
      "FF FF FF FF 00 06 00 FE", "FF FF FF F4 00 06 00 FE", "FF FF FF F9 00 06 00 FE", "FF FF FF FE 00 06 00 FE",
      "FF FF FF F3 00 06 00 FE", "FF FF FF F8 00 06 00 FE", "FF FF FF FD 00 06 00 FE", "FF FF FF F2 00 06 00 FE",
      "FF FF FF F7 00 06 00 FE", "FF FF FF FC 00 06 00 FE", "FF FF FF F1 00 06 00 FE", "FF FF FF F6 00 06 00 FE",
      "00 00 42 DB 00 62 90 FE", "00 00 62 70 00 96 90 FE", "00 00 70 E5 00 74 90 FE", "00 00 77 7A 00 37 90 FE",
      "00 00 7A 8F 00 06 90 FE", "00 00 7B E4 00 4A 90 FE", "00 00 7C 89 00 93 90 FE", "00 00 7C CE 00 2F 90 FE",
      "00 00 7C E3 00 71 90 FE", "00 00 7C F8 00 5E 90 FE", "00 00 7D 0D 00 6A 90 FE", "00 00 7D 02 00 6A 90 FE"};
  uint8_t out[8];
  for (uint32_t s = 0; s < 24; s++) {
    build_frame(d, BUS_ZU, s, 0, 0, out);
    char buf[40];
    snprintf(buf, sizeof(buf), "%02X %02X %02X %02X %02X %02X %02X %02X", out[0], out[1], out[2], out[3], out[4],
             out[5], out[6], out[7]);
    EXPECT_STREQ(buf, want[s]) << s;
  }
}

TEST(TwingoBus, Frame19fIgnitionOneRampsBytesZeroAndOneUpToA0) {
  const FrameDef& d = *find_frame(0x19F);
  uint8_t out[8];
  build_frame(d, BUS_ZUENDUNG1, 100, 0, 0, out, 12);
  EXPECT_EQ(out[6], 0x90);
  EXPECT_EQ(out[0], 0x00);
  build_frame(d, BUS_ZUENDUNG1, 101, 0, 0, out, 13);
  EXPECT_EQ(out[6], 0x50);
  EXPECT_EQ(out[0], 0x00);
  build_frame(d, BUS_ZUENDUNG1, 200, 0, 0, out, 114);
  EXPECT_EQ(out[0], 0x00);
  build_frame(d, BUS_ZUENDUNG1, 201, 0, 0, out, 115);
  EXPECT_EQ(out[0], 0x01);
  EXPECT_EQ(out[1], 0x01);
  build_frame(d, BUS_ZUENDUNG1, 202, 0, 0, out, 125);
  EXPECT_EQ(out[0], 0x18);
  build_frame(d, BUS_ZUENDUNG1, 203, 0, 0, out, 146);
  EXPECT_EQ(out[0], 0xA0);
  EXPECT_EQ(out[1], 0xA0);
  build_frame(d, BUS_ZUENDUNG1, 204, 0, 0, out, Z1_19F_RAMP_DONE);
  EXPECT_EQ(out[0], 0xA0);
  EXPECT_EQ(out[1], 0xA0);
  EXPECT_EQ(out[2], 0x7D);
  EXPECT_EQ(out[6], 0x50);
}

TEST(TwingoBus, Frame0ecCounterAndCrcAreConsistent) {
  const FrameDef& d = *find_frame(0x0EC);
  uint8_t out[8];
  for (uint32_t s = 0; s < 40; s++) {
    build_frame(d, BUS_WACH, s, 0, 0, out);
    EXPECT_EQ(out[1] >> 4, s & 0x0F);
    EXPECT_EQ(out[2], (uint8_t)(crc8(out, 2, 0x1D) ^ 0xBE)) << s;
  }
}

TEST(TwingoBus, Frame157CounterStepsFive) {
  const FrameDef& d = *find_frame(0x157);
  uint8_t out[8];
  build_frame(d, BUS_WACH, 0, 0, 0, out);
  EXPECT_EQ(out[1] >> 4, 0x0);
  build_frame(d, BUS_WACH, 1, 0, 0, out);
  EXPECT_EQ(out[1] >> 4, 0x5);
  build_frame(d, BUS_WACH, 7, 0, 0, out);
  EXPECT_EQ(out[1] >> 4, 35 & 0x0F);
}

TEST(TwingoBus, Frame511SixAccumulatorsMatchTheFirstTwoRealFrames) {
  const FrameDef& d = *find_frame(0x511);
  uint8_t out[8];
  build_frame(d, BUS_WACH, 0, 0, 0, out);
  const uint8_t f0[6] = {0x3A, 0x15, 0xC1, 0xF9, 0x53, 0x97};
  const uint8_t f1[6] = {0x79, 0xDE, 0x7A, 0x64, 0x60, 0x7A};  // second frame of the log
  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(out[1 + i], f0[i]) << i;
  }
  build_frame(d, BUS_WACH, 1, 0, 0, out);
  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(out[1 + i], f1[i]) << i;
  }
}

TEST(TwingoBus, Frame511Byte0FollowsTheState) {
  const FrameDef& d = *find_frame(0x511);
  uint8_t out[8];
  build_frame(d, BUS_WACH, 0, 0, 0, out);
  EXPECT_EQ(out[0], 0x04);
  build_frame(d, BUS_FAHRT, 0, 0, 0, out);
  EXPECT_EQ(out[0], 0x00);
}

TEST(TwingoBus, Frame426HasStateBytesAndTwentyFourBitKilometres) {
  const FrameDef& d = *find_frame(0x426);
  uint8_t out[8];
  build_frame(d, BUS_WACH, 0, 0, 6844, out);  // 6844 = 0x1ABC
  const uint8_t wach[8] = {0x00, 0x00, 0x06, 0x00, 0x1A, 0xBC, 0x00, 0x40};
  for (int i = 0; i < 8; i++) {
    EXPECT_EQ(out[i], wach[i]) << i;
  }
  build_frame(d, BUS_FAHRT, 0, 0, 6844, out);
  EXPECT_EQ(out[1], 0x70);
  EXPECT_EQ(out[2], 0x69);
  build_frame(d, BUS_ZU, 0, 0, 65536 + 5, out);  // 24 bit: 72,... km needs byte 3
  EXPECT_EQ(out[2], 0x02);
  EXPECT_EQ(out[3], 0x01);
  EXPECT_EQ(out[4], 0x00);
  EXPECT_EQ(out[5], 0x05);
  build_frame(d, BUS_ZU, 0, 0, 72380, out);  // pack reading of 10.10.: 0x011ABC = 72,380 km came from 01 1A BC
  EXPECT_EQ(out[3], 0x01);
  EXPECT_EQ(out[4], 0x1A);
  EXPECT_EQ(out[5], 0xBC);
  build_frame(d, BUS_ZU, 0, 0, 92784, out);  // the car log: 01 6A 70
  EXPECT_EQ(out[3], 0x01);
  EXPECT_EQ(out[4], 0x6A);
  EXPECT_EQ(out[5], 0x70);
  build_frame(d, BUS_ZUENDUNG1, 0, 0, 0, out);
  EXPECT_EQ(out[1], 0x60);
  EXPECT_EQ(out[2], 0x65);
  build_frame(d, BUS_GO, 0, 0, 0, out);
  EXPECT_EQ(out[2], 0x61);
  build_frame(d, BUS_AUS, 0, 0, 0, out);
  EXPECT_EQ(out[2], 0x05);
}

TEST(TwingoBus, Frame436CarriesTheAgeInBytesOneToThree) {
  const FrameDef& d = *find_frame(0x436);
  uint8_t out[8];
  build_frame(d, BUS_WACH, 0, 1025301, 0, out);  // 1,025,301 = 0x0FA515
  EXPECT_EQ(out[0], 0x80);
  EXPECT_EQ(out[1], 0x0F);
  EXPECT_EQ(out[2], 0xA5);
  EXPECT_EQ(out[3], 0x15);
  EXPECT_EQ(out[4], 0x00);
  EXPECT_EQ(out[5], 0x00);
  build_frame(d, BUS_ZUENDUNG1, 0, 1025301, 0, out);
  EXPECT_EQ(out[0], 0xAD);
  build_frame(d, BUS_AUS, 0, 1025301, 0, out);
  EXPECT_EQ(out[0], 0x80);
}

TEST(TwingoBus, Frame423AlternatesTheTwoVariantsEveryFiveFrames) {
  const FrameDef& d = *find_frame(0x423);
  uint8_t a[8], b[8];
  build_frame(d, BUS_WACH, 0, 0, 0, a);
  build_frame(d, BUS_WACH, 4, 0, 0, b);
  for (int i = 0; i < 8; i++) {
    EXPECT_EQ(a[i], b[i]);
  }
  build_frame(d, BUS_WACH, 5, 0, 0, b);
  EXPECT_NE(a[4], b[4]);  // bytes 4/6 swap between 0x5D and 0xB2
  EXPECT_NE(a[6], b[6]);
  EXPECT_TRUE((a[4] == 0x5D && b[4] == 0xB2) || (a[4] == 0xB2 && b[4] == 0x5D));
  build_frame(d, BUS_WACH, 10, 0, 0, b);
  for (int i = 0; i < 8; i++) {
    EXPECT_EQ(a[i], b[i]);
  }
}

TEST(TwingoBus, Frame500OnlyWhileTheIgnitionIsOn) {
  const FrameDef& d = *find_frame(0x500);
  EXPECT_FALSE(active_in_state(d, BUS_ZU));
  EXPECT_FALSE(active_in_state(d, BUS_WACH));
  EXPECT_TRUE(active_in_state(d, BUS_ZUENDUNG1));
  EXPECT_TRUE(active_in_state(d, BUS_FAHRT));
  EXPECT_TRUE(active_in_state(d, BUS_AUS));
  EXPECT_TRUE(active_in_state(*find_frame(0x423), BUS_ZU));
}

TEST(TwingoBus, Frame155InEveryStateIsAValidFrameForTheDriver) {
  const FrameDef& d = *find_frame(0x155);
  uint8_t out[8];
  for (uint8_t st = 0; st < BUS_STATE_COUNT; st++) {
    build_frame(d, st, 0, 0, 0, out);
    const uint16_t current_raw = (uint16_t)(((out[1] & 0x0F) << 8) | out[2]);
    const uint16_t soc_raw = (uint16_t)((out[4] << 8) | out[5]);
    EXPECT_TRUE(twingo::frame_155_valid(current_raw, soc_raw)) << state_name(st);
  }
}

TEST(TwingoBus, Frame69fCarriesTheVehicleIdInEveryState) {
  const FrameDef& d = *find_frame(0x69F);
  uint8_t out[8];
  for (uint8_t st = 0; st < BUS_STATE_COUNT; st++) {
    build_frame(d, st, 0, 0, 0, out);
    EXPECT_EQ(out[0], 0x46);
    EXPECT_EQ(out[1], 0x13);
    EXPECT_EQ(out[2], 0x88);
    EXPECT_EQ(out[3], 0x6F);
  }
}

TEST(TwingoBus, OutOfRangeStateFallsBackToAwake) {
  uint8_t a[8], b[8];
  build_frame(*find_frame(0x426), 99, 0, 0, 100, a);
  build_frame(*find_frame(0x426), BUS_WACH, 0, 0, 100, b);
  for (int i = 0; i < 8; i++) {
    EXPECT_EQ(a[i], b[i]);
  }
}

}  // namespace

// ---------------------------------------------------------------------------
// State sequence (11.10.): wake-up of session 3 and shutdown of session 1 of the car log 22aaf176
// ---------------------------------------------------------------------------

TEST(TwingoBusSequence, WakeUpWalksTheStatesOfTheCarAndStaysAtDriving) {
  struct {
    uint32_t t;
    uint8_t state;
    int32_t o426;
  } want[] = {{0, BUS_ZU, -1},
              {1199, BUS_ZU, -1},
              {1200, BUS_WACH, 0x0802},
              {1899, BUS_WACH, 0x0802},
              {1900, BUS_WACH, 0x0806},
              {2299, BUS_WACH, 0x0806},
              {2300, BUS_WACH, -1},
              {13099, BUS_WACH, -1},
              {13100, BUS_ZUENDUNG1, 0x6067},
              {13600, BUS_ZUENDUNG1, -1},
              {15100, BUS_ZUENDUNG3, -1},
              {16800, BUS_GO, -1},
              {18700, BUS_FAHRT, 0x7061},
              {19900, BUS_FAHRT, -1},
              {600000, BUS_FAHRT, -1}};
  for (const auto& w : want) {
    int32_t o = 123;
    EXPECT_EQ(wake_state(w.t, o), w.state) << "t=" << w.t;
    EXPECT_EQ(o, w.o426) << "t=" << w.t;
  }
}

TEST(TwingoBusSequence, ShutdownFromDrivingWalksDownToAwake) {
  struct {
    uint32_t t;
    uint8_t state;
    int32_t o426;
  } want[] = {{0, BUS_ZUENDUNG1, 0x7065}, {99, BUS_ZUENDUNG1, 0x7065}, {100, BUS_ZUENDUNG1, -1},
              {199, BUS_ZUENDUNG1, -1},   {200, BUS_AUS, -1},          {2099, BUS_AUS, -1},
              {2100, BUS_AUS, 0x0007},    {4699, BUS_AUS, 0x0007},     {4700, BUS_WACH, -1},
              {120000, BUS_WACH, -1}};
  for (const auto& w : want) {
    int32_t o = 123;
    EXPECT_EQ(sleep_state(w.t, BUS_FAHRT, o), w.state) << "t=" << w.t;
    EXPECT_EQ(o, w.o426) << "t=" << w.t;
  }
}

TEST(TwingoBusSequence, ShutdownFromAnyOtherStateStartsAtIgnitionOff) {
  int32_t o = 123;
  EXPECT_EQ(sleep_state(0, BUS_WACH, o), BUS_AUS);
  EXPECT_EQ(o, -1);
  EXPECT_EQ(sleep_state(1900, BUS_GO, o), BUS_AUS);
  EXPECT_EQ(o, 0x0007);
  EXPECT_EQ(sleep_state(4500, BUS_ZU, o), BUS_WACH);
  EXPECT_EQ(o, -1);
}

TEST(TwingoBusSequence, Frame426TakesTheTransitionalValuesAndByte7) {
  const FrameDef& d = *find_frame(0x426);
  uint8_t out[8];
  build_frame(d, BUS_WACH, 0, 0, 6844, out, 0, 0x0802);  // wake-up step 2: 00 08 02 00 1A BC 00 40
  const uint8_t step2[8] = {0x00, 0x08, 0x02, 0x00, 0x1A, 0xBC, 0x00, 0x40};
  for (int i = 0; i < 8; i++) {
    EXPECT_EQ(out[i], step2[i]) << i;
  }
  build_frame(d, BUS_AUS, 0, 0, 92784, out, 0, 0x0007);  // 00 00 07 01 6A 70 00 40 as in the car
  const uint8_t aus7[8] = {0x00, 0x00, 0x07, 0x01, 0x6A, 0x70, 0x00, 0x40};
  for (int i = 0; i < 8; i++) {
    EXPECT_EQ(out[i], aus7[i]) << i;
  }
  build_frame(d, BUS_WACH, 0, 0, 6844, out, 0, -1, 0x55);  // byte 7 from the field
  EXPECT_EQ(out[7], 0x55);
  EXPECT_EQ(out[1], 0x00);
  EXPECT_EQ(out[2], 0x06);
}
