//////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007-2017 musikcube team
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the author nor the names of other contributors may
//      be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.hpp"
#include "PersistedPlayQueueQuery.h"
#include <core/db/Statement.h>
#include <core/db/ScopedTransaction.h>
#include <core/library/track/TrackList.h>

using namespace musik::core;
using namespace musik::core::db;
using namespace musik::core::db::local;

PersistedPlayQueueQuery::PersistedPlayQueueQuery(
    musik::core::ILibraryPtr library,
    musik::core::audio::PlaybackService& playback,
    Type type)
: library(library)
, playback(playback)
, type(type)
{

}

PersistedPlayQueueQuery::~PersistedPlayQueueQuery() {

}

bool PersistedPlayQueueQuery::OnRun(musik::core::db::Connection &db) {
    ScopedTransaction transaction(db);

    if (this->type == Type::Save) {
        TrackList tracks(this->library);
        this->playback.CopyTo(tracks);

        {
            Statement deleteTracks("DELETE FROM last_session_play_queue", db);
            deleteTracks.Step();
        }

        {
            Statement insert("INSERT INTO last_session_play_queue (track_id) VALUES (?)", db);
            for (size_t i = 0; i < tracks.Count(); i++) {
                insert.Reset();
                insert.BindInt64(0, tracks.GetId(i));
                insert.Step();
            }
        }
    }
    else if (this->type == Type::Restore) {
        auto editor = this->playback.Edit();
        editor.Clear();

        Statement query("SELECT track_id FROM last_session_play_queue ORDER BY id ASC", db);
        while (query.Step() == db::Row) {
            editor.Add(query.ColumnInt64(0));
        }
    }

    return true;
}