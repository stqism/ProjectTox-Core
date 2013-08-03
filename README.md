![Project Tox](https://rbt.asia/boards/g/img/0352/79/1373823047559.png "Project Tox")
Project Tox, _also known as Tox_, is a FOSS (Free and Open Source Software) instant messaging application aimed to replace Skype.<br />

With the rise of governmental monitoring programs, Tox aims to be an easy to use, all-in-one communication platform (including audio, and videochats in the future) that ensures their users full privacy and secure message delivery.<br /> <br />



**IRC**: #tox on freenode, alternatively, you can use the [webchat](http://webchat.freenode.net/?channels=#tox).<br />
**Website**: [http://tox.im](http://tox.im)
**Developer Blog**: [http://dev.tox.im](http://dev.tox.im)

**Website translations**: [see stal888's repository](https://github.com/stal888/ProjectTox-Website)<br/>
**Qt GUI**: [see nurupo's repository](https://github.com/nurupo/ProjectTox-Qt-GUI)

**How to build Tox on Linux**: [YouTube video](http://www.youtube.com/watch?v=M4WXE4VKmyg)<br />
**How to use Tox on Windows**: [YouTube video](http://www.youtube.com/watch?v=qg_j_sDb6WQ)<br />
**For Mac OSX read** [INSTALL.md](INSTALL.md)

### Objectives:

Keep everything really simple.

## The Complex Stuff:
+ Tox must use UDP simply because [hole punching](http://en.wikipedia.org/wiki/UDP_hole_punching) with TCP is not as reliable.
+ Every peer is represented as a [byte string][String] (the public key of the peer [client ID]).
+ We're using torrent-style DHT so that peers can find the IP of the other peers when they have their ID.
+ Once the client has the IP of that peer, they start initiating a secure connection with each other. (See [Crypto](https://github.com/irungentoo/ProjectTox-Core/wiki/Crypto))
+ When both peers are securely connected, they can exchange messages, initiate a video chat, send files, etc, all using encrypted communications.
+ Current build status: [![Build Status](https://travis-ci.org/irungentoo/ProjectTox-Core.png?branch=master)](https://travis-ci.org/irungentoo/ProjectTox-Core)

## Roadmap:
- [x] Get our DHT working perfectly. (Done, needs large scale testing though)
- [x] Reliable connection (See Lossless UDP protocol) to other peers according to client ID. (Done, see `DHT_sendfiletest.c` for an example)
- [x] Encryption. (Done)
- [  ] Get a simple text only IM client working perfectly. (This is where we are)
- [  ] Streaming media
- [  ] ???

For further information, check our [To-do list](http://wiki.tox.im/index.php/TODO)

### Why are you doing this? There are already a bunch of free skype alternatives.
The goal of this project is to create a configuration-free P2P skype 
replacement. Configuration-free means that the user will simply have to open the program and 
without any account configuration will be capable of adding people to his 
friends list and start conversing with them. There are many so-called skype replacements and all of them are either hard to 
configure for the normal user or suffer from being way too centralized.

### Documentation:

- [Installation](/INSTALL.md)
- [Commands](/docs/commands.rst)
- [DHT Protocol](https://github.com/irungentoo/ProjectTox-Core/wiki/DHT)<br />
- [Lossless UDP Protocol](https://github.com/irungentoo/ProjectTox-Core/wiki/Lossless-UDP)<br />
- [Crypto](https://github.com/irungentoo/ProjectTox-Core/wiki/Crypto)<br />
- [Ideas](https://github.com/irungentoo/ProjectTox-Core/wiki/Ideas)

[String]: https://en.wikipedia.org/wiki/String_(computer_science)
