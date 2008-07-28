// Meta header for OpenAL components.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENENGINE_OPENAL_H_
#define _OPENENGINE_OPENAL_H_

#if defined __APPLE__
    #include <alc.h>
    #include <al.h>
#else
    #include <AL/alc.h>
    #include <AL/al.h>
#endif

#endif // _OPENENGINE_OPENAL_H_
