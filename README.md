# Reverse Engineered Vape Circuit Boards

Disposable vapes show up all the time in e-waste and are often found as litter at the side of the road (shame!) but they contain some useful components that can be reused by the cost-conscious hacker.

*Note: Wear gloves when handling used devices and don't allow the vape liquid to contact your skin or eyes.*

I've reverse engineered a number of different brands and placed the schematics and layout files in this repository. The summary table below is a good starting point.

Many of the vapes have debugging connections accessible through the USB-C charging cable interface, typically SWD on the CC pins, so you may be able to reprogram the vape without even opening up the case.

| Brand | PCB ID | Display | MCU | NOR flash | Battery IC | LDO regulator | Debug | Other Notes |
|-------|--------|---------|-----|-----------|------------|---------------|-------|-------------|
| (Unknown) | [FT03 V3.0](FT03/) | LCD | CHIPSEA F031K8V6 | GT 25Q32A | PC3221 | CJ6330 | SWD on CC lines. UART test points. | Touch IC TTP223. Pushbutton. 3 heater outputs with feedback. |
| Flum Mello | [TMY\_0118470101\_V04](TMY/) | LCD | CHIPSEA F031K8V6 | ZB 25VQ32 | TP4054 | CJ6330 | SWD on CC lines. | One heater output with feedback. |
| Rama 16000 | [YK761-VP9012-A](YK761/) | LCD | BlueX BX2400 or RF01 | Puya PY25Q64 | CST4056 | CJ6330 | Test points for SWD | One heater with feedback. Bluetooth (!) |
| GeekBar Pulse 15000 | [BD0027-USB\_V1.5](BD0027/) | Blue and RGB LEDs | Puya PY32F030EK28 | None | LP4068 | None | SWD on CC lines. | 3-position slide switch. Two heaters with feedback. |
| Lost Mary MO20000 Pro | [STW2915 V1.7](STW2915/) | LCD | Pya PY32F030EK28 | Puya PY32Q64 | LP4068 | CJ6330 (?) | SWD on CC lines. | |
| GeekBar Pulse X Jam | [BD0061](BD0061/) | LCD and blue LEDs, PLS916H driver IC | Puya PY32F030EK28 | None | LR5112/LR5108 | None | SWD on CC lines. | |

I also have [firmware running on the Lost Mary MO20000 Pro](STW2915/firmware) that could be modified to run on other vapes with the same PY32F030 microcontroller.

## License

All the reverse engineered hardware data is provided under the [Creative Commons Attribution-ShareAlike 4.0 International](https://creativecommons.org/licenses/by-sa/4.0/) license.

