#!/bin/bash

APP_NAME="Ledit"                   
EXECUTABLE="./Release/ledit"         
ICON_NAME="AppIcon"                  
SOURCE_ICON="./assets/logo.png" 
DESTINATION_DIR="macos_app"             

# Create the app bundle structure
mkdir -p "$DESTINATION_DIR/$APP_NAME.app/Contents/MacOS"
mkdir -p "$DESTINATION_DIR/$APP_NAME.app/Contents/Resources"

# Copy the executable
cp "$EXECUTABLE" "$DESTINATION_DIR/$APP_NAME.app/Contents/MacOS/$APP_NAME"
chmod +x "$DESTINATION_DIR/$APP_NAME.app/Contents/MacOS/$APP_NAME"

# Create an Info.plist file
cat > "$DESTINATION_DIR/$APP_NAME.app/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>$APP_NAME</string>
    <key>CFBundleIconFile</key>
    <string>$ICON_NAME</string>
    <key>CFBundleIdentifier</key>
    <string>net.liz3.$APP_NAME</string>
    <key>CFBundleName</key>
    <string>$APP_NAME</string>
    <key>CFBundleVersion</key>
    <string>1.0.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
</dict>
</plist>
EOF

# Generate the ICNS file from the PNG
mkdir "$ICON_NAME.iconset"
sips -z 16 16     "$SOURCE_ICON" --out "$ICON_NAME.iconset/icon_16x16.png"
sips -z 32 32     "$SOURCE_ICON" --out "$ICON_NAME.iconset/icon_16x16@2x.png"
sips -z 32 32     "$SOURCE_ICON" --out "$ICON_NAME.iconset/icon_32x32.png"
sips -z 64 64     "$SOURCE_ICON" --out "$ICON_NAME.iconset/icon_32x32@2x.png"
sips -z 128 128   "$SOURCE_ICON" --out "$ICON_NAME.iconset/icon_128x128.png"
sips -z 256 256   "$SOURCE_ICON" --out "$ICON_NAME.iconset/icon_128x128@2x.png"
sips -z 256 256   "$SOURCE_ICON" --out "$ICON_NAME.iconset/icon_256x256.png"
sips -z 512 512   "$SOURCE_ICON" --out "$ICON_NAME.iconset/icon_256x256@2x.png"
sips -z 512 512   "$SOURCE_ICON" --out "$ICON_NAME.iconset/icon_512x512.png"
sips -z 1024 1024 "$SOURCE_ICON" --out "$ICON_NAME.iconset/icon_512x512@2x.png"
iconutil -c icns "$ICON_NAME.iconset" -o "$DESTINATION_DIR/$APP_NAME.app/Contents/Resources/$ICON_NAME.icns"
rm -R "$ICON_NAME.iconset"

echo "App bundle created at $DESTINATION_DIR/$APP_NAME.app"
