# M24SR64-Y
Arduino library to support the dynamic NFC/RFID Tag IC dual interface M24SR64-Y

## Limitation

From I2C interface, you can't write tag with a total size (protocol + payload)
greater than 246 bytes. This is the limitation of the Iblock command on M24SR device.

## Examples

The library includes six sketches. They are very similar. Each sketch has a different type of tag.
The WriteAAR sketch writes a AAR (Android Application Record) tag on the device. It opens an application on your smartphone.
The WriteMime sketch writes a mime tag on the device. It records a define type of data.
The WriteSMS sketch writes a SMS tag on the device. It records a SMS body and a recipient phone number.
The WriteURI sketch writes a URI tag on the device. It records an URI.
The WriteURIMail sketch writes a Mail tag on the device. It records a mail with the recipient, the subject and the body of the message.
the WriteText sketch writes a Text tag on the device. It records a simple text message.

When the NFC module is started and ready, the message "Sytstem init done!" is displayed on the monitor window.
Next, the tag is written, we wait few seconds, we read the same tag and print it on the monitor window.

You can test this application by connecting it with your smartphone.
On Android, donwload a NFC Tools. Then start the app, check if NFC is activated
on your smartphone. Put your smartphone near the tag, you can read it. You can
write a tag with this app.

## API

Before the setup :
Choose the pinout of the I2C to use for communicate with the NFC device. Choose the pinout of the M24SR64-Y.

In the setup :
The M24SR64-Y uses I2C.
  dev_i2c.begin();

Start the NFC module.
  nfcTag.begin(NULL);

In the loop :

Write the tag.
  nfcTag.writeTxt(text_write);

Read the tag.
  nfcTag.readTxt(text_read);

## Version

The initial NFC API comes from x-nucleo-nfc01a1 (28 April 2017).
Version = 2.0.0

## Documentation

You can find the source files at  
https://github.com/stm32duino/M24SR64-Y

The M24SR64-Y datasheet is available at  
http://www.st.com/en/nfc/m24sr64-y.html
