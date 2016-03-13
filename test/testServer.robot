*** Settings ***
Library  Remote  http://localhost:8080
 
*** Test Cases ***

Ping Test
   ${pong} =                        ping  bob 
   Should Be Equal as Strings       ${pong}           pong bob

Objects test
   ${objs} =   read all objects
 
Click object
   click   pushButton

#Server State Test
#   Server Status Should be          AdminServer       RUNNING
 
#Application State Test
#   Application Status Should Be     oracleRobot       ACTIVE
 
#Data Source Existence Test
#   Data Source Should Exist         myDataSource
