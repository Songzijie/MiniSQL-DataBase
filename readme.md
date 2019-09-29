# MiniSQL

     __  __ _       _ ____   ___  _  
    |  \/  (_)_ __ (_) ___| / _ \| |  
    | |\/| | | '_ \| \___ \| | | | |  
    | |  | | | | | | |___) | |_| | |___  
    |_|  |_|_|_| |_|_|____/ \__\_\_____|  

## Description
This is a MiniSql engine powered by c++, DBMS homework.

## Author
Zijie Song

## How to Use
1. g++ src/BufferManager.cpp src/CatalogManager.cpp src/RecordManager.cpp src/main.cpp src/Interpreter.cpp src/helpers.cpp src/Index.cpp -Iinclude -std=c++11 -o minisql
2. ./minisql
