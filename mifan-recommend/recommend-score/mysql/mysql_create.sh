#!/bin/bash

# host=10.30.100.15;
host=10.1.1.65;
user=elvis;
pwd=elvispwd;

curDb=db_newsfeed_recommend
createSql="create database if not exists $curDb default charset=utf8"
mysql -h$host -u$user -p$pwd -e "$createSql"

mtable=table.sql
echo "creating $curDb passive tables....";
mysql -h$host -u$user -p$pwd $curDb < "$mtable"
