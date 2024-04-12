### Prerequisites

Create a fork of https://github.com/friyiajr/BLESampleExpo/tree/main

Install java version 11
```bash
curl -s "https://get.sdkman.io" | bash
sdk install java 11.0.12-zulu
```

Install watchman
```bash
brew install watchman
```

### Actions

Clone the fork to your development directory

```bash
cd BLESampleExpo
```

```bash
rm -rf android node_modules
```

```bash
npm install
npx expo install expo-dev-client
npx expo install --fix
npx expo-doctor@latest
```

```bash
npx npm install eas-cli
npx expo prebuild --clean --platform android
```

```bash
echo "sdk.dir=/Users/XXX/Library/Android/sdk" > android/local.properties
```

```bash
npx expo run:android
```

