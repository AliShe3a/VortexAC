ALTER PROCEDURE [dbo].[ANTI_CHEAT_AUTH_INSERT]
  @Usn AS bigint,
  @User_ID AS varchar(50),
  @User_Pass AS varchar(50),
  @User_name AS varchar(256),
  @Domain_name AS varchar(256),
  @Reg1 AS varchar(1024),
  @Reg2 AS varchar(1024),
  @Time AS varchar(42),
  @Client_IP AS varchar(256)
AS
BEGIN
  IF EXISTS (SELECT 1 FROM [dbo].[ANTICHEAT_HARDWARE_BAN] WHERE REG1 = @Reg1)
      OR EXISTS (SELECT 1 FROM [dbo].[ANTICHEAT_HARDWARE_BAN] WHERE REG2 = @Reg2)
      OR EXISTS (SELECT 1 FROM [dbo].[ANTICHEAT_HARDWARE_BAN] WHERE USERNAME = @User_name)
      OR EXISTS (SELECT 1 FROM [dbo].[ANTICHEAT_HARDWARE_BAN] WHERE DOMAIN_NAME = @Domain_name)
      OR EXISTS (SELECT 1 FROM [dbo].[ANTICHEAT_HARDWARE_BAN] WHERE CLIENT_IP = @Client_IP)
  BEGIN

 Set @Time = '0'
 
  END
  ELSE
  BEGIN
    INSERT INTO dbo.ANTICHEAT_AUTH
    (
      Usn,
      User_id,
      User_pass,
      User_name,
      Domain_name,
      Registry1,
      Registry2,
      Time,
      Client_IP
    )
    VALUES
    (
      @Usn,
      @User_ID,
      @User_Pass,
      @User_name,
      @Domain_name,
      @Reg1,
      @Reg2,
      @Time,
      @Client_IP
    );
  END
END