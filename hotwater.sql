# database schema
CREATE DATABASE hotwater ;
USE hotwater ;
CREATE TABLE hotwater ( 
  Id INT NOT NULL AUTO_INCREMENT, 
  Timestamp DATETIME NOT NULL,
  TankUpper DOUBLE NOT NULL,
  TankLower DOUBLE NOT NULL,
  Outside DOUBLE,
  Notes TEXT,
  PRIMARY KEY(Id) ) ;

  
