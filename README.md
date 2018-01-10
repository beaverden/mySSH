# mySSH

## Introduction
mySSH is a minimalistic cryptographic network protocol for remote control of a system over an unsecured network. A mySSH server will listen on a machine waiting for a number of clients to connect over a netwrok and securely trasmit commands. The commands (also called command lines) will have a specific format and the server will ensure the correct execution and forwading of the output to the client.
Installation

## Documentation
Please read mySSH.pdf in the docs directory

## Dependencies 
  1. Install OpenSSL on your distribution

## Installation
  1. In the main directory, run `make -B server`
  2. In the main directory, run `make -B client`
  3. In the main directory, run `make -B shell`

## Usage
### Server
  1. Make bin folder the current directory and run server with the port to run. 
  2. Wait for clients to connect and be served
  Example: `sudo ./server 2018`

  
### Client 
  1. Run the client using ip, port and username.
  Example: `./client 127.0.0.1 2018 denis`
