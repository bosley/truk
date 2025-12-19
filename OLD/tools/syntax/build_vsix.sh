#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

VERSION="0.1.0"
VSIX_NAME="truk-language-${VERSION}.vsix"

rm -f "$VSIX_NAME"
rm -rf .vsix_build

mkdir -p .vsix_build/extension
mkdir -p .vsix_build/extension/syntaxes

cp package.json .vsix_build/extension/
cp language-configuration.json .vsix_build/extension/
cp icon.png .vsix_build/extension/
cp syntaxes/truk.tmLanguage.json .vsix_build/extension/syntaxes/

cat > .vsix_build/extension.vsixmanifest << 'EOF'
<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011" xmlns:d="http://schemas.microsoft.com/developer/vsx-schema-design/2011">
  <Metadata>
    <Identity Language="en-US" Id="truk-language" Version="0.1.0" Publisher="truk"/>
    <DisplayName>Truk Language Support</DisplayName>
    <Description xml:space="preserve">Syntax highlighting for the Truk programming language</Description>
    <Tags>truk,programming,language</Tags>
    <Categories>Programming Languages</Categories>
    <Icon>extension/icon.png</Icon>
  </Metadata>
  <Installation>
    <InstallationTarget Id="Microsoft.VisualStudio.Code"/>
  </Installation>
  <Dependencies/>
  <Assets>
    <Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" Addressable="true"/>
    <Asset Type="Microsoft.VisualStudio.Services.Icons.Default" Path="extension/icon.png" Addressable="true"/>
  </Assets>
</PackageManifest>
EOF

cat > .vsix_build/'[Content_Types].xml' << 'EOF'
<?xml version="1.0" encoding="utf-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension=".json" ContentType="application/json"/>
  <Default Extension=".png" ContentType="image/png"/>
  <Default Extension=".vsixmanifest" ContentType="text/xml"/>
</Types>
EOF

cd .vsix_build
zip -r "../$VSIX_NAME" . -q

cd ..
rm -rf .vsix_build

echo "Created $VSIX_NAME"
