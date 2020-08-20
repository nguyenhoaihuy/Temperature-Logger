# Temperature Logger

## Table of Contents

## Introduction

This project is about getting to know how TCP and TLS protocols work. By working with BeagleBone Green Kit, we understand how IOT interact with each other.

## Build Status

[![Build Status](https://travis-ci.com/travis-ci/travis-web.svg?branch=master)](https://travis-ci.com/travis-ci/travis-web)

## Requirement

1. [`BeagleBone`](#https://www.mouser.com/ProductDetail/Seeed-Studio/110060426?qs=Zwj7mHVHPHTyD4oKZvExkg%3D%3D&gclid=Cj0KCQjwvvj5BRDkARIsAGD9vlIRRKNLDuiFXxnJrm7zRGTJdOPXbTqGme6986jSV_TqGl_imTsQAPUaAkWZEALw_wcB) Wireless IOT developer prototyping kit

2. [`Debian`](#https://www.debian.org/) image latest version

## Installation

1. Flash Debian image to the BeagleBone by following the [link](https://medium.com/@shreeya.patel23498/getting-started-with-beaglebone-green-309b6718aa02)

2. Setup Environment for BeagleBone (wifi, ssh) by following the [link](https://support.thingplus.net/en/open-hardware/bbb-user-guide.html)

3. Connect Temperature Sensor to the A0 analog input on the cape

![image](https://developer.atmosphereiot.com/images/Guides/GettingStartedBeagleBoneGreen/BeagleBoneGreenSetup.jpg)

## Usage

1. Clone this repo to local machine

2. Use SCP to copy tcp.c and tls.c to BeagleBone

3. SSH to BeagleBone

4. *cd \<dir_name\>*

5. *make default*  

6. *./tcp \<your_host_name\>*

7. *./tls \<your_host_name\>*

## Contribute option

One of the disabvantage of this project is that the server side has to be running on tcp protocol with tls authetication  

## Contribute

1. Fork it

2. Create your feature branch

3. Commit your changes

4. Push to the branch

5. Create a new Pull Request
