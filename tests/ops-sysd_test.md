ops-sysd Test Cases
===================

Contents
--------
- [Image manifest read test](#image-manifest-read-test)
- [H/W description file read test](#hardware-description-files-read-test)
- [/etc/os-release file read test](#/etc/os-release-file-read-test)


Image manifest read test
------------------------
### Objective ###
Verify that sysd correctly processes the image.manifest file.

### Requirements ###
Virtual Mininet Test Setup.

### Setup ###
#### Topology Diagram ####
```
  [s1]
```

### Description ###
Bring up ops-sysd with various image.manifest file and verify that daemon info
in the database matches what is in image.manifest file.

### Test Result Criteria ###
#### Test Pass Criteria ####
1. Check if ops-sysd changes the some of the hardware handler to false.
1. Check if ops-sysd changes the management interface from eth0 to mgmt1.
1. Check if ops-sysd behaves correctly even with the random information in the
   file.

#### Test Fail Criteria ####
One or more verifications fail.


Hardware description files read test
-----------------------------------------
### Objective ###
Verify that ops-sysd correctly processes the hardware description files.

### Requirements ###
Virtual Mininet Test Setup.

### Setup ###
#### Topology Diagram ####
```
  [s1]
```

### Description ###
Bring up ops-sysd with various hardware description files and checks if it
corredctly populates those informations in the appropriate table/columns in
the database.

### Test Result Criteria ###
#### Test Pass Criteria ####
1. Verify number\_ports is correct
1. Verify max\_bond\_count is correct
1. Verify max\_lag\_member\_count is correct
1. Verify switch\_device\_port is correct
1. Verify connector is correct
1. Verify bridge\_normal is correct
1. Verify vrf\_default is correct

#### Test Fail Criteria ####
One of the verification fails, e.g. the number of ports are not correct.


/etc/os-release file read test
------------------------------
### Objective ###
Verify that ops-sysd correctly processes the /etc/os-release file.

### Requirements ###
Virtual Mininet Test Setup.

### Setup ###
#### Topology Diagram ####
```
  [s1]
```

### Description ###
1. Copy the sample os-releases files to the VSI switch /tmp directory
1. Stop the OVSDB server as well as ops-sysd on the switch
1. Copy the specific os-release file, e.g os-release.ops-1.0.0,
   to the /etc/os-release file.
1. Start the OVSDB server as well as ops-sysd on the switch
1. Verify that the software\_info.os\_name as well as
   switch\_version column of the System table in OVSDB shows the
   corresponding information stored in the /etc/os-release file.

### Test Result Criteria ###
#### Test Pass Criteria ####
- Verify OS name in the OVSDB is same with the appropriate /etc/os-release NAME entry
- Verify switch version in the OVSDB is same with the appropriate /etc/os-release VERSION\_ID and BUILD\_ID.

#### Test Fail Criteria ####
One of the verification fails, e.g. OS name in the OVSDB is different from the /etc/os-release NAME value.
