use strict;
use Data::Dumper;
use File::Basename;
use Getopt::Long;

my $output_pic;
my $output_plist;
my $output_proj;
my @inputFiles;

GetOptions("output-proj=s" => \$output_proj
           , "output-pic=s" => \$output_pic
           , "output-plist=s" => \$output_plist
           , "input=s" => \@inputFiles
           )
  or die "参数格式错误";

sub get_project_default;

my $project;

if ( -e $output_proj ) {
  if ( open my $fh, '<', $output_proj ) {
    $project = do { local $/; <$fh> };
    close $fh;
  }
}

if ( ! defined $project ) {
  $project = <<END;
<?xml version="1.0" encoding="UTF-8"?>
<data version="1.0">
    <struct type="Settings">
        <key>fileFormatVersion</key>
        <int>1</int>
        <key>variation</key>
        <string>main</string>
        <key>verbose</key>
        <false/>
        <key>autoSDSettings</key>
        <array/>
        <key>allowRotation</key>
        <false/>
        <key>quiet</key>
        <false/>
        <key>premultiplyAlpha</key>
        <false/>
        <key>shapeDebug</key>
        <false/>
        <key>dpi</key>
        <uint>72</uint>
        <key>dataFormat</key>
        <string>cocos2d</string>
        <key>textureFileName</key>
        <filename>dd</filename>
        <key>flipPVR</key>
        <false/>
        <key>ditherType</key>
        <enum type="SettingsBase::DitherType">NearestNeighbour</enum>
        <key>backgroundColor</key>
        <uint>0</uint>
        <key>libGdx</key>
        <struct type="LibGDX">
            <key>filtering</key>
            <struct type="LibGDXFiltering">
                <key>x</key>
                <enum type="LibGDXFiltering::Filtering">Linear</enum>
                <key>y</key>
                <enum type="LibGDXFiltering::Filtering">Linear</enum>
            </struct>
        </struct>
        <key>shapePadding</key>
        <uint>2</uint>
        <key>jpgQuality</key>
        <uint>80</uint>
        <key>pngOptimizationLevel</key>
        <uint>0</uint>
        <key>textureSubPath</key>
        <string></string>
        <key>textureFormat</key>
        <enum type="SettingsBase::TextureFormat">png</enum>
        <key>borderPadding</key>
        <uint>2</uint>
        <key>maxTextureSize</key>
        <QSize>
            <key>width</key>
            <int>2048</int>
            <key>height</key>
            <int>2048</int>
        </QSize>
        <key>fixedTextureSize</key>
        <QSize>
            <key>width</key>
            <int>-1</int>
            <key>height</key>
            <int>-1</int>
        </QSize>
        <key>reduceBorderArtifacts</key>
        <false/>
        <key>algorithmSettings</key>
        <struct type="AlgorithmSettings">
            <key>algorithm</key>
            <enum type="AlgorithmSettings::AlgorithmId">MaxRects</enum>
            <key>freeSizeMode</key>
            <enum type="AlgorithmSettings::AlgorithmFreeSizeMode">Best</enum>
            <key>sizeConstraints</key>
            <enum type="AlgorithmSettings::SizeConstraints">POT</enum>
            <key>forceSquared</key>
            <false/>
            <key>forceWordAligned</key>
            <false/>
            <key>maxRects</key>
            <struct type="AlgorithmMaxRectsSettings">
                <key>heuristic</key>
                <enum type="AlgorithmMaxRectsSettings::Heuristic">Best</enum>
            </struct>
            <key>basic</key>
            <struct type="AlgorithmBasicSettings">
                <key>sortBy</key>
                <enum type="AlgorithmBasicSettings::SortBy">Best</enum>
                <key>order</key>
                <enum type="AlgorithmBasicSettings::Order">Ascending</enum>
            </struct>
        </struct>
        <key>andEngine</key>
        <struct type="AndEngine">
            <key>minFilter</key>
            <enum type="AndEngine::MinFilter">Linear</enum>
            <key>packageName</key>
            <string>Texture</string>
            <key>javaFileName</key>
            <filename></filename>
            <key>wrap</key>
            <struct type="AndEngineWrap">
                <key>s</key>
                <enum type="AndEngineWrap::Wrap">Clamp</enum>
                <key>t</key>
                <enum type="AndEngineWrap::Wrap">Clamp</enum>
            </struct>
            <key>magFilter</key>
            <enum type="AndEngine::MagFilter">MagLinear</enum>
        </struct>
        <key>dataFileName</key>
        <filename></filename>
        <key>multiPack</key>
        <false/>
        <key>mainExtension</key>
        <string></string>
        <key>forceIdenticalLayout</key>
        <false/>
        <key>outputFormat</key>
        <enum type="SettingsBase::OutputFormat">RGBA8888</enum>
        <key>contentProtection</key>
        <struct type="ContentProtection">
            <key>key</key>
            <string></string>
        </struct>
        <key>autoAliasEnabled</key>
        <true/>
        <key>trimSpriteNames</key>
        <false/>
        <key>globalSpriteSettings</key>
        <struct type="SpriteSettings">
            <key>scale</key>
            <double>1</double>
            <key>scaleMode</key>
            <enum type="ScaleMode">Smooth</enum>
            <key>innerPadding</key>
            <uint>0</uint>
            <key>extrude</key>
            <uint>0</uint>
            <key>trimThreshold</key>
            <uint>1</uint>
            <key>trimMode</key>
            <enum type="SpriteSettings::TrimMode">Trim</enum>
            <key>heuristicMask</key>
            <false/>
        </struct>
        <key>fileList</key>
        <array></array>
        <key>ignoreFileList</key>
        <array/>
        <key>replaceList</key>
        <array/>
        <key>ignoredWarnings</key>
        <array/>
        <key>commonDivisorX</key>
        <uint>1</uint>
        <key>commonDivisorY</key>
        <uint>1</uint>
    </struct>
</data>
END

}

$project =~ s/(\<key\>textureFileName\<\/key\>\s+\<filename\>)([^<]*)(\<\/filename\>)/$1$output_pic$3/ms or die "replace output pic fail";
$project =~ s/(\<key\>dataFileName\<\/key\>\s+\<filename\>)([^<]*)(\<\/filename\>)/$1$output_plist$3/ms or die "replace output plist fail";

my $project_file_list = join('', map { "\n            <filename>$_</filename>" } @inputFiles) . "\n        ";
$project =~ s/(\<key\>fileList\<\/key\>\s+\<array\>)(.*)(\<\/array\>)/$1$project_file_list$3/ms or die "replace file list fail";

open(my $out, ">$output_proj") or die "create output file $output_proj fail!";

print $out $project;

