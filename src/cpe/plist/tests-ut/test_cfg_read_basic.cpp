#include "CfgTest.hpp"

TEST_F(CfgTest, map_basic) {
    EXPECT_EQ(
        0, read(
            "<plist version='1.0'>\n"
            "    <dict>\n"
            "        <key>frames</key>\n"
            "        <dict>\n"
            "            <key>fight_effect_37/01.png</key>\n"
            "            <dict>\n"
            "                <key>frame</key>\n"
            "                <string>{{232,690},{138,212}}</string>\n"
            "                <key>offset</key>\n"
            "                <string>{-131,-19}</string>\n"
            "                <key>rotated</key>\n"
            "                <true/>\n"
            "                <key>sourceColorRect</key>\n"
            "                <string>{{0,38},{138,212}}</string>\n"
            "                <key>sourceSize</key>\n"
            "                <string>{400,250}</string>\n"
            "            </dict>\n"
            "         </dict>\n"
            "    </dict>\n"
            "</plist>\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "frames:\n"
        "    fight_effect_37/01.png:\n"
        "        frame: '{{232,690},{138,212}}'\n"
        "        offset: '{-131,-19}'\n"
        "        rotated: !uint8 1\n"
        "        sourceColorRect: '{{0,38},{138,212}}'\n"
        "        sourceSize: '{400,250}'\n"
        "...\n"
        , result());
}
