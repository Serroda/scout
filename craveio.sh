#!/bin/bash
set -e

echo "<Crave.io build for motorola edge 60 fusion (scout) with CRDROID - Android 16>"

echo "1. Remove old manifest"
rm -rf .repo/local_manifests

echo "2. Init Crdorid repo"
repo init -u https://github.com/crdroidandroid/android.git -b 16.0 --git-lfs --no-clone-bundle

echo "3. Download scout files"
rm -rf ./scoutfiles ./device/motorola ./vendor/motorola
git clone --depth=1 --branch=crdroid16 https://github.com/Serroda/scout scoutfiles
cd ./scoutfiles
git lfs pull
cd ..
cp -rf ./scoutfiles/device .
cp -rf ./scoutfiles/vendor .
rm -rf ./scoutfiles

echo "4. Copy new local_manifest inside .repo"
cp -fr device/motorola/scout/local_manifests .repo/

echo "5. Resync dependencies"
/opt/crave/resync.sh

echo "5.1. Patch check_boot_jars allowed list"
ALLOWED_LIST="build/soong/scripts/check_boot_jars/package_allowed_list.txt"
if ! grep -F -q 'com\.motorola' "$ALLOWED_LIST"; then
    printf '\n# Moto adds\ncom\\.motorola\ncom\\.motorola\\..*\n' >> "$ALLOWED_LIST"
    echo "-> Rules com.motorola added to $ALLOWED_LIST"
fi

echo "6. Setup env"
source build/envsetup.sh

echo "7. Build"
brunch scout

echo "<--Build finished-->"
