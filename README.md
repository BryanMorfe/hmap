# HMAP

## Short Description
HMAP is a simply Unix/Linux utility that allows you to add, remove, or show your local DNS entries. It is a very simple utility made to simplify the life of the user by hiding underlying details about the local DNS entries, protects the user from making mistakes such as having duplicate records or having an incorrect format, and easily view the current local DNS entries on the system. Note, as the license states, this software is provided AS IS, with NO GUARANTEES of any kind.

## TODO
- Finish implementation operation `rmhost`.
- Validate provided IP addresses and hosts for maps and `spec-host`.
- Add operation to validate host files.
- Add the ability to add multiple maps from a file.
- Refine output message by giving user more details on to what's wrong.

## Usage

### Basic

#### Display Local DNS Records
`$ hmap`  
An example map is:
```
127.0.0.1 <- localhost,
192.168.1.1 <- myrouter,
192.168.1.33 <- node33,
192.168.1.34 <- node34,
```  
And this means that domain/host `localhost` points to IP address `127.0.0.1`, etc.

#### Adding a New Record
Adding a new record mean to add a host(s) to IP map, i.e. when you use the domain names below, it will point to the IP `192.168.1.50`.  
`$ sudo hmap -o addhost --map "192.168.1.50 <- www.example.com, example.com, example.net, www.example.net"`

#### Removing an Existing Record
As of right now, only search by hosts/domains are possible. In the below example, we will remove the map `192.168.1.50 <- www.example.com`. Note that `example.com`, `example.net`, and `www.example.net` will still point to `192.168.1.50`:  
`$ sudo hmap -o rmhost -s www.example.com`  

#### Help Menu
For more options and information, execute:  
`$ hmap --help`

### Advanced
This section is made for those that know a bit of the underlying process behind hosts file. HMAP gives you the ability to provide custom hosts files to modify. If you'd like to inspect what HMAP does, this is a good option, or if you simply would prefer not to modify your original hosts file for any reason, then you can provide a custom hosts file. You may see `$ hmap --help` for information on how to provide custom files, as well as everything you can do with them.

## Installation
You can download a released version, or you may compile from source. To compile from source, simply obtain the source code of this repo:  
`$ git clone https://github.com/BryanMorfe/hmap.git`  
Then compile:  
`$ make`  
Install:
`$ sudo make install`  
