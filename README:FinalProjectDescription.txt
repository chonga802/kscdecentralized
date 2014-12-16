BitTorrent with Chord Table trackers

“BitTorrEnnant” - Christine Hong, Kojiro Murase, Sage Price

PROJECT DESCRIPTION:
We propose to construct a file sharing system that uses the BitTorrent protocol for downloading, where seed tracking is controlled by a Chord protocol DHT. BitTorrent gives us the benefits of improved speed, non-sequential download, and reduced burden on individual nodes during download. Using Chord for seed tracking reduces overhead, and provides O(log(T)) search time for files, where T is the size of the table.


Responsibilities:
    Sage- Chord tracking of seeds
    Christine - BitTorrent download
    Koji - GUI, reputation, proof-of-work entry


===================================
PART 1: HIGH-LEVEL OVERVIEW

    Joining the network - users enter an ID and neighbor whose network they wish to join. Upon entering the ID, the program will append integers to the ID until an ID String is generated that hashes to a specified value. This provides a barrier-to-entry which reduces the probability of Sybil attacks.

    Download - files are downloaded non-sequentially from the set of nodes in the network who already have the file saved locally to disc. These nodes are referred to as “seeders” for that file. Non-sequential download means that if a node wishes to download, it collects all of the file’s metadata from a tracker node, then randomly selects blocks to request from the various seeders. They continue to request random blocks until the entire file has been obtained. At this point they save the file locally and send a download report to the tracker for a file. This report contains two pieces of info: that this node is now seeding the file, and the data on how many blocks were obtained from each seeder. This data is used to calculate reputation scores for future downloads of that file.

    Tracking - all nodes in the network function as trackers. What a node tracks is the metadata for any file that hashes to a location between the node and its predecessor in the table. Thus if node A tracks file f1, then A locally stores the hash of each block of f1, the metafile ID, number of blocks in f1, and those nodes who are seeds for f1. If someone wishes to download a file, they must first send a seed request to the tracker for that file. This means that they look up the nearest predecessor of the file they have in their chord table, who forward the request on as in the Chord protocol. This is repeated until the file reaches the tracker, who replies to the original requester with all the metadata and seed information, at which point requestor messages seeders to initiate download.

    Reputation - reputation is a simple calculation of blocks successfully downloaded from each seeder when downloading a file. This means that it is a peer-based calculation, and is applied at the file level. Reputation is then used by nodes when downloading to decide who is the most reliable source to message for data.

    Network-wide data - in order to facilitate ease of use and download, trackers periodically will broadcast messages to the network at large announcing what files they have, as long as the number of seeders for each file. This makes it easy to see what files are available for download to users on the network. Furthermore, all nodes in the network must be aware of all other nodes in order to keep consistent finger tables, so periodic route rumors are broadcast and used to construct and maintain these tables.


===================================
PART 2: PROCESS DESCRIPTIONS

Chord Table desc:
    all nodes are trackers
    files are owned by node after them on Chord table
    each node has finger table

Tracker desc:
    owns files that hash to between it and predecessor in DHT
    files contain the following:
    metafileID
    blocklist hash
    seeds (list of users with file)
    reps of seeds (in same order as seeds, add this last)

Upload Process:
    Create all hashes for file
    Send rumor out: file and metafileID
    Tell tracker file exists, inform them you are seed

Download process (get seeds):
    search my files for keyword matches
    choose file to dl
    do Chord table search messaging (described lower down)
    Await seed response

Download process (w/ seeds):
    Select seeds (based on rep, do this at end)
    Pick random not yet downloaded block
    Send block request to chosen seed
    Await response
    Verify response and request new random block
    If timeout, pick new seed to request random blocks from
    When all blocks obtained, concatenate and save file

Chord table search:
    check if you have data
    if so, send seed response
    else go to step 2
    find tracker for file in finger table (call getTracker)
    send seed request to tracker

Finger Table deets:
    size: 256 entries (2^8)
    so finger table has 8 entries
    finger table contains successor of location (n + 2^i) mod(2^8),
        where n is entry of node and i is entry in finger table

Seeds:
    Contain the following (all that is needed for dl):
        file name
        metafile ID
        blocklist hash
    On first upload, sends following out to tracker:
        file name
        metafile ID
        blocklist hash
        seeder (should be done via ORIGIN field of message)
    

==========================================
PART 3: API’S and CLASS DESCRIPTIONS

Chord API:
    constructor - takes a QString, the node’s ID

    bool updateFingers(QString id)-
        checks if peer named id should be in finger table, adds it if so
        returns true to indicate new predecessor -> might need to share files
        returns false to indicate no new predecessor

    int getLocation (QString id)-  returns the hash table entry corresponding to the given id

    QString getTracker (QString filename) - given a filename and metafile, returns corresponding entry in finger table; this is either the tracker for the node, or nearest predecessor you know

Message Control in Main:
    void sendBlockRequestToSeeders(QVariantMap)
        when seed reply is processed, send block requests to seeders until everything is received
    sendBlockRequest(int blockNum, QString seeder)
        sends block request to seeder
    void sendBlockReply(QVariantMap)
        when block request is received, send part of file you have to requester
    void processBlockReply(QVariantMap)
        add blocks to collection (remove from blocksLeft as you process them)
        if file is still not complete, repeat sendBlockRequestToSeeders



Chord Class Miscellanea:
    Fields:
        myLoc: int, node’s index in table
        myID: QString, node’s id
        predLoc: int, node’s predecessor in table
        predID: QString, predecessor’s id
        finger[ ]: QString, array of fingers; currently size 8, as table is 2^8
    Initial State:
        id is set to self, predecessor is set to self, location is calculated and set
        all fingers are self (no other nodes, so node is successor by defn)

Tracker stuff
    TrackedFileMetaData class
        Includes file metadata + list of seeders
    Functions (in main chatDialog)
        sendBroadcast()
            sends broadcast of your tracked files


==================================
PART 4: NEW MESSAGES

Seeders Request:
    Origin - QString
    Dest - QString
    SeedRequest - QString

Seeders Response:
    Origin - QString
    Dest - QString
    SeedReply - QString
    MetaFileID - QByteArray
    Seeders - QStringList
    BlockListHash - QByteArray

Upload Notice: (To inform network of file availability, done once at first upload)
    contains files and number of seeds

Block Request: (modified from old Peerster)
    Dest - QString
    Origin - QString
    BlockRequest - QByteArray
    BlockNum - quint32

Block Reply: (modified from old Peerster)
    Dest - QString
    Origin - QString
    BlockReply - QByteArray
    Data - QByteArray
    BlockNum - quint32

