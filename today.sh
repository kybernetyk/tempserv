#!/bin/sh

sqlite3 temp.db "select time(timestamp, 'unixepoch', 'localtime') as Datum, temp as Temperatur from data where timestamp >= strftime('%s', date('now'), 'utc');"
