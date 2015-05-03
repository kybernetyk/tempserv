#!/bin/sh
sqlite3 temp.db "select temp from data order by timestamp desc limit 1;"

