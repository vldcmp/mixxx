
#include <QtDebug>
#include <QtCore>
#include <QSqlQuery>
#include <QSqlError>
#include "trackinfoobject.h"
#include "library/dao/playlistdao.h"

PlaylistDAO::PlaylistDAO(QSqlDatabase& database)
        : m_database(database) {

}

PlaylistDAO::~PlaylistDAO()
{
}

void PlaylistDAO::initialize()
{
    //Start the transaction
    QSqlDatabase::database().transaction();
    
    QSqlQuery query;
    query.exec("CREATE TABLE Playlists (id INTEGER primary key, "
               "name varchar(48), position INTEGER, "
               "date_created datetime, "
               "date_modified datetime)");
    //query.finish();

    query.exec("CREATE TABLE PlaylistTracks (id INTEGER primary key, "
               "playlist_id INTEGER REFERENCES Playlists(id),"
               "track_id INTEGER REFERENCES library(id), "
               "position INTEGER)");
               
    QSqlDatabase::database().commit();
}

/** Create a playlist with the given name.
    @param name The name of the playlist to be created.
*/
void PlaylistDAO::createPlaylist(QString name)
{

    //Start the transaction
    QSqlDatabase::database().transaction();

    //Find out the highest position for the existing playlists so we know what
    //position this playlist should have.
    QSqlQuery query;
    query.prepare("SELECT (position) FROM Playlists "
                  "ORDER BY position DESC");
    query.exec();
    //query.finish();

    //Get the id of the last playlist.
    int position = 0;
    if (query.next()) {
        position = query.value(query.record().indexOf("position")).toInt();
        position++; //Append after the last playlist.
    }

    qDebug() << "inserting playlist" << name << "at position" << position;

    query.prepare("INSERT INTO Playlists (name, position) "
                  "VALUES (:name, :position)");
 				 // ":date_created, :date_modified)");
    query.bindValue(":name", name);
    query.bindValue(":position", position);
    query.exec();
    //query.finish();

    //Start the transaction
    QSqlDatabase::database().commit();

    qDebug() << query.lastQuery();

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }
    /*
    query.prepare("SELECT FROM Playlists (id)"
              "WHERE name=(:name)");
    query.bindValue(":name", name);
    query.exec(); */

    /*
    //Get the id of the newly created playlist.
    query.prepare("SELECT last_insert_rowid()");
    query.exec();

    int id = -1;
    while (query.next()) {
        id = query.value(query.record().indexOf("id")).toInt();
    }*/


    return;
}

/** Find out the name of the playlist at the given position */
QString PlaylistDAO::getPlaylistName(unsigned int position)
{
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    query.prepare("SELECT (name) FROM Playlists "
                  "WHERE position=(:position)");
    query.bindValue(":position", position);
    query.exec();

    QSqlDatabase::database().commit();

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << "getPlaylistName:" << query.lastError();
     	return "";
    }

    //Get the name field
    QString name;
    query.next();
    name = query.value(query.record().indexOf("name")).toString();
    //query.finish();

    return name;
}

int PlaylistDAO::getPlaylistIdFromName(QString name) {
    QSqlQuery query;
    query.prepare("SELECT id FROM Playlists WHERE name=:name");
    query.bindValue(":name", name);
    if (query.exec()) {
        if (query.next()) {
            return query.value(query.record().indexOf("id")).toInt();
        }
    } else {
        qDebug() << "getPlaylistIdFromName:" << query.lastError();
    }
    return -1;
}


/** Delete a playlist */
void PlaylistDAO::deletePlaylist(int playlistId)
{
    QSqlDatabase::database().transaction();

    //Get the playlist id for this
    QSqlQuery query;

    //Delete the row in the Playlists table.
    query.prepare("DELETE FROM Playlists "
                  "WHERE id=(:id)");
    query.bindValue(":id", playlistId);
    if (!query.exec()) {
     	qDebug() << "deletePlaylist" << query.lastError();
     	return;
    }
    //query.finish();

    //Delete the tracks in this playlist from the PlaylistTracks table.
    query.prepare("DELETE FROM PlaylistTracks "
                  "WHERE playlist_id=(:id)");
    query.bindValue(":id", playlistId);
    if (!query.exec()) {
     	qDebug() << "deletePlaylist" << query.lastError();
     	return;
    }
    //query.finish();

    QSqlDatabase::database().commit();
    //TODO: Crap, we need to shuffle the positions of all the playlists?
}

/** Append a track to a playlist */
void PlaylistDAO::appendTrackToPlaylist(int trackId, int playlistId)
{
    qDebug() << "appendTrackToPlaylist, track:" << trackId << "playlist:" << playlistId;

    //Start the transaction
    QSqlDatabase::database().transaction();

    //Find out the highest position existing in the playlist so we know what
    //position this track should have.
    QSqlQuery query;
    query.prepare("SELECT (position) FROM PlaylistTracks "
              "WHERE playlist_id=(:id) "
              "ORDER BY position DESC");
    query.bindValue(":id", playlistId);
    query.exec();

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << "appendTrackToPlaylist" << query.lastError();
     	//return;
    }

    //TODO: RJ suggestions
    //select max(position) as position from PlaylistTracks;
    //where playlist_id = this_playlist

    //Get the position of the highest playlist...
    int position = 0;
    if (query.next()) {
        position = query.value(query.record().indexOf("position")).toInt();
    }
    position++; //Append after the last song.


    //Insert the song into the PlaylistTracks table
    query.prepare("INSERT INTO PlaylistTracks (playlist_id, track_id, position)"
                  "VALUES (:playlist_id, :track_id, :position)");
    query.bindValue(":playlist_id", playlistId);
    query.bindValue(":track_id", trackId);
    query.bindValue(":position", position);
    query.exec();

    //Start the transaction
    QSqlDatabase::database().commit();
}

/** Find out how many playlists exist. */
unsigned int PlaylistDAO::playlistCount()
{
    QSqlQuery query;
    query.prepare("SELECT * FROM Playlists");
    query.exec();

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }

 	int numRecords = 0;
    while (query.next()) {
 		numRecords++;
    }

    //qDebug() << numRecords << "playlists found.";

    return numRecords;
}

int PlaylistDAO::getPlaylistId(int position)
{
    //Find out the highest position existing in the playlist so we know what
    //position this track should have.
    QSqlQuery query;
    query.prepare("SELECT (id) FROM Playlists "
              "WHERE position=(:position)");
    query.bindValue(":position", position);
    query.exec();

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
     	return -1;
    }

    //Get the id field
    int playlistId;
    query.next();
    playlistId = query.value(query.record().indexOf("id")).toInt();
    //query.finish();

    return playlistId;
}

void PlaylistDAO::removeTrackFromPlaylist(int playlistId, int position)
{
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    //Delete the track from the playlist.
    query.prepare("DELETE FROM PlaylistTracks "
                  "WHERE playlist_id=(:id) AND position=(:position)");
    query.bindValue(":id", playlistId);
    query.bindValue(":position", position);
    query.exec();
    //query.finish();

    QString queryString;
    queryString = QString("UPDATE PlaylistTracks SET position=position-1 "
                  "WHERE position>=%1 AND "
                  "playlist_id=%2").arg(position).arg(playlistId);
    query.exec(queryString);
    //query.finish();
    QSqlDatabase::database().commit();
}

void PlaylistDAO::insertTrackIntoPlaylist(QString location, int playlistId, int position)
{
    //FIXME stub!
    qDebug() << "STUB: TrackCollection::insertTrackIntoPlaylist";
}
