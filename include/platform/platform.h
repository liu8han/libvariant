/* 
 *  Copyright (c) 2010,
 *  Gavriloaie Eugen-Andrei (shiretu@gmail.com)
 * 
 *  Copyright (c) 2011,
 *  Han Liu (cn.liuhan@gmail.com)
 * 
 *
 *  This file is changed from crtmpserver (http://www.rtmpd.com/)
 *  crtmpserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  crtmpserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with crtmpserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PLATFORM_H
#define _PLATFORM_H

#define LITTLE_ENDIAN_BYTE_ALIGNED
#include "./endianess/endianness.h"

#ifdef OSX
#include "./osx/osxplatform.h"
#define Platform OSXPlatform
#endif /* OSX */

#ifdef LINUX
#include "./linux/linuxplatform.h"
#define Platform LINUXPlatform
#endif /* LINUX */

#endif /* _PLATFORM_H */
