name: Build Geode Mod

on:
  push:
    branches: ["main", "master"]
  pull_request:
    branches: ["main", "master"]
  workflow_dispatch:

jobs:
  build:
    strategy:
      fail-fast: false
      matrix:
        config:
          - name: Android32
            os: ubuntu-latest
            target: Android32

          - name: Android64
            os: ubuntu-latest
            target: Android64

    name: ${{ matrix.config.name }}
    runs-on: ${{ matrix.config.os }}

    steps:
      - uses: actions/checkout@v4

      - name: Build the mod
        uses: geode-sdk/build-geode-mod@main
        with:
          # Ép hệ thống tự động gộp thành file .geode riêng cho từng nền tảng ngay tại đây
          combine: true
          target: ${{ matrix.config.target }}

      # Tải trực tiếp file .geode lên làm Artifact ngay sau khi build xong từng nền tảng
      - name: Upload Mod Package
        uses: actions/upload-artifact@v4
        with:
          name: Show-Position-${{ matrix.config.target }}
          path: ./**/*.geode
          if-no-files-found: error
