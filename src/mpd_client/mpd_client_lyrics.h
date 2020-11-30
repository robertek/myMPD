/*
 SPDX-License-Identifier: GPL-2.0-or-later
 myMPD (c) 2018-2020 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

#ifndef __MPD_CLIENT_LYRICS_H__
#define __MPD_CLIENT_LYRICS_H__
sds mpd_client_handle_lyrics(t_mpd_client_state *mpd_client_state, sds buffer, sds method, long request_id, const char *uri);
#endif
