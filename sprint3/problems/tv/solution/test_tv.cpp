#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "tv.h"

// Тесты для класса TV
TEST(TV, IsOffByDefault) {
    TV tv;
    EXPECT_FALSE(tv.IsTurnedOn());
}

TEST(TV, DoesntShowChannelWhenOff) {
    TV tv;
    EXPECT_FALSE(tv.GetChannel().has_value());
}

TEST(TV, TurnsOn) {
    TV tv;
    tv.TurnOn();
    EXPECT_TRUE(tv.IsTurnedOn());
    EXPECT_THAT(tv.GetChannel(), testing::Optional(1));
}

TEST(TV, TurnsOff) {
    TV tv;
    tv.TurnOn();
    tv.TurnOff();
    EXPECT_FALSE(tv.IsTurnedOn());
    EXPECT_FALSE(tv.GetChannel().has_value());
}

TEST(TV, SelectChannel) {
    TV tv;
    tv.TurnOn();
    tv.SelectChannel(5);
    EXPECT_THAT(tv.GetChannel(), testing::Optional(5));
}

TEST(TV, SelectChannelOnlyInRange) {
    TV tv;
    tv.TurnOn();
    tv.SelectChannel(0);
    EXPECT_THAT(tv.GetChannel(), testing::Optional(1));
    tv.SelectChannel(1000);
    EXPECT_THAT(tv.GetChannel(), testing::Optional(1));
    tv.SelectChannel(500);
    EXPECT_THAT(tv.GetChannel(), testing::Optional(500));
}

TEST(TV, SelectPreviousChannel) {
    TV tv;
    tv.TurnOn();
    tv.SelectChannel(5);
    tv.SelectChannel(7);
    tv.SelectPreviousChannel();
    EXPECT_THAT(tv.GetChannel(), testing::Optional(5));
    tv.SelectPreviousChannel();
    EXPECT_THAT(tv.GetChannel(), testing::Optional(7));
}

TEST(TV, SelectPreviousChannelWhenOff) {
    TV tv;
    tv.SelectPreviousChannel();  // Должно игнорироваться
    tv.TurnOn();
    EXPECT_THAT(tv.GetChannel(), testing::Optional(1));
}

TEST(TV, RemembersChannelAfterTurnOff) {
    TV tv;
    tv.TurnOn();
    tv.SelectChannel(42);
    tv.TurnOff();
    tv.TurnOn();
    EXPECT_THAT(tv.GetChannel(), testing::Optional(42));
}
