#!/bin/sh

sqlite3 temp.db "select datetime(timestamp, 'unixepoch', 'localtime') as Datum, temp as Temperatur from data;"
