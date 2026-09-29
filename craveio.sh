#!/bin/bash
set -e

echo "<Crave.io build for motorola edge 60 fusion (scout) with Infinity X - Android 16>"

echo "1. Remove old manifest"
rm -rf .repo/local_manifests

echo "2. Init Infinity X repo"
repo init --depth=1 --no-repo-verify --git-lfs -u https://github.com/ProjectInfinity-X/manifest -b 16 -g default,-mips,-darwin,-notdefault

echo "3. Download scout files"
curl -sSL https://gitlab.com/Serroda/scout_v/-/archive/main/scout_v-main.tar.gz | tar -xz --strip-components=1

echo "4. Copy new local_manifest inside .repo"
cp -fr device/motorola/scout/local_manifests .repo/

echo "5. Resync dependencies"
/opt/crave/resync.sh

echo "6. Setup env"
source build/envsetup.sh

echo "7. Build"
lunch infinity_scout-userdebug && m bacon

echo "<--Build finished-->"
