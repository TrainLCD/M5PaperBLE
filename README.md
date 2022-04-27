# TrainLCD/M5PaperBLE

M5Paper を TrainLCD の外部ディスプレイにするプロジェクト

## これなに

https://twitter.com/tinykitten8/status/1495088649712861184

このツイートを見てすべてを察してくれ

## どうやって使うの

- このコードをおおむろに M5Paper に焼く
  - `SERVICE_UUID`、 `CHARACTERISTIC_UUID`、`BLE_DEVICE_NAME` は変えたほうがいいかも
- https://github.com/TrainLCD/MobileApp のリポジトリを clone するとたぶん `feature/ble` というブランチがあると思うのでチェックアウトして起動
  - このリポジトリ側で `SERVICE_UUID`、`CHARACTERISTIC_UUID`、`BLE_DEVICE_NAME` を変えたらアプリ側ハードコードされている `TARGET_SERVICE_UUID`、`TARGET_CHARACTERISTIC_UUID`、`BLE_DEVICE_NAME` を変更してね
- M5Paper が TrainLCD の外部ディスプレイになる。やったね
