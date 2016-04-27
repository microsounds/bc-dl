## bc-dl - basic CLI downloader for bandcamp.com
![version] ![license]

```bc-dl``` is a minimal command line music scraping application for downloading 128kbps MP3 streams from any [bandcamp.com](http://bandcamp.com) album page. ```bc-dl``` is written in pure *C89*, making it extremely lightweight and fast. With few external dependencies (ONE), it should run on any POSIX-compliant system.

> ```bc-dl``` stands for _"Basic CLI Downloader"_. It does **not** stand for _"Bandcamp Downloader"_.

## Features
* ```bc-dl``` takes a URL to a Bandcamp album page and downloads all available ```mp3-128``` streams into your current directory.
* Albums will be saved in the format ```Artist - Album Name (20XX)/01. Track.mp3```.
* Accurate ID3v2.4 tags will be written to each track, along with full size album artwork.
* Supports UTF-8 encoding.
* If interrupted, downloads can continue where you left off.
* You can also provide a list of newline-separated URLs and ```bc-dl``` will iterate through them non-interactively.

### Note
This program is primarily for ripping low quality copies of paid albums, _(i.e. streaming copies available on their page)._

If a particular album is available as _"Pay what you want"_ and the artist has not run out of free downloads this month, then you should download a full quality copy through Bandcamp directly, because this program will **not** attempt to do it for you.

Support your favorite artists by purchasing the official, high quality release whenever possible.

> ```bc-dl``` is not affiliated with Bandcamp or bandcamp.com.

## Usage
```
usage:
	bc-dl [-h | -v | -i list.txt] http://artist.bandcamp.com/album/example
help:
	-h (--help) - Display this help screen.
	-v (--version) - Version and license information.
	-i (--iterate) - Provide iterated list of urls.
```

## Building
```git clone``` into this repository, run ```make``` to build it.
Run ```sudo make install``` if you wish to install it on your system.

You can also grab the latest stable version as a zip or tarball from the [Releases](https://github.com/microsounds/bc-dl/releases) tab. 

This program uses ```libcurl``` and POSIX Regular Expressions.

Make sure your environment has these installed before continuing.

## Issues
Bandcamp likes to change their JSON layout periodically, feel free to open an issue if you notice any problems.

## License/Copyright
```
bc-dl - basic CLI downloader for bandcamp.com
Copyright (C) 2016 microsounds <https://github.com/microsounds>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
```
[version]: https://img.shields.io/badge/version-v0.5.4-brightgreen.svg?style=flat)
[license]: https://img.shields.io/badge/license-GPLv3-red.svg?style=flat)
