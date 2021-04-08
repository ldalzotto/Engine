# Asset database

The asset database is a database that stores :

1. Engine assets as blob from human readable format
2. Engine assets dependencies
3. Engine assets file metadata (compilation only, never requested by the engine)

<svg-inline src="asset_database_architecture.svg"></svg-inline>

All assets are identified by a unique id that is the hash code of the local path of the asset file. <br/>

For example, if we specify at database compilation time that the asset root is "E:/document/project/assets/" then an asset file located at "E:/document/project/assets/model/test.obj" will be stored and requested by using "model/test.obj" as input for hash calculation.

Hashes are calculated by using the djp2 function ([hash](http://www.cse.yorku.ca/~oz/hash.html)).

## Engine assets

Engine assets are stored as a blob chunk. The goal of the database is to provide an object format that needs the least amount of calculation for the engine to interpret it. <br/>
So assets files are transformed from a human readable format (or commonly used file extensions like .png, .obj) to one that is close to the engine object.

For exemple, if we want to store an array of integers, the json array :

```json
{
    "array":[0,1,2,3,4,5]
}
```
will be compiled to it's packed memory representation :

```
[size] (size_t 8 bytes)
[0] (int 4 bytes)
[1] (int 4 bytes)
[2] (int 4 bytes)
[3] (int 4 bytes)
[4] (int 4 bytes)
```

All custom defined asset files are written in json. The first field of every json file must be the type of the asset file. <br/>
For exemple, if we define a material, the json file will start with : 
```json
{
    "type": "MATERIAL",
    ...
}
```

## Engine asset dependencies

Asset dependencies are used to idicate all assets that are mentionned by the current asset. The system of compilation to blob is exactly the same as assets. The difference is that we are compiling the asset to provide a list of dependant asset ids.

Asset dependencies can be recursively evaluated, it is up the asset compilation implementation to decide it. Again, the goal is to be the most efficient possible when dependencies are read by the engine.

The assets dependencies have their own table because :

1. We don't always request them, because we have already requested an asset dependency that have been evaluated recursively.
2. In the future, we can have a case where we want to request dependies in different format for the same asset.

## Engine asset metadata

Asset metadata are all data that are irrelevant to the engine and only used or updated by tools.

# Asset compilation

The asset compilation is the program executed to transform human readable assets to engine assets. <br/>

The program takes an asset configuration json file that lists all assets that are going to be compiled. <br/>
All assets path mentionned are relative to the asset folder root.

The asset compilation configuration can be used to compile multiple asset databases. This is useful when writing test cases for example.

The configuration file has a "common" section that is used to compile asset that are required for the engine to startup. The required assets will be compiled for all database.