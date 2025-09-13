

## What is this ?

This is a tiny temparature recorder with CR2302 x 2 batteries.

## Commands

Using `digiterm`, `r` shows recorded values. First line is number of recorded values.

```
$ sudo digiterm
Looking for Digispark running DigiUSB...
Attached to <Digispark:DigiUSB:@2.29>
Type control + c to exit
r
5
29.54
29.04
28.82
29.22
29.71
```

`c` clears recorded values.

```
$ sudo digiterm
Looking for Digispark running DigiUSB...
Attached to <Digispark:DigiUSB:@2.29>
Type control + c to exit
c
ok

r
1
29.15
```
