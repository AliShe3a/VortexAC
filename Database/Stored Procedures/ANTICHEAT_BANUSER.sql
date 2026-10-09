ALTER PROCEDURE [dbo].[ANTICHEAT_BANUSER]
@USN AS bigint ,
@ERROR_CODE AS varchar( MAX ),
@USER_ID AS varchar( MAX ),
@USER_PASS AS varchar( MAX),
@WARNING bigint
AS
BEGIN

-- Check the WARNING parameter
IF @WARNING = 1
BEGIN

    EXEC CF_SA_WEB_DB.dbo.WSP_USER_BAN  @USN , 2,0,'A','ANTI_CHEAT',3,'fghfgh',1,0

END
ELSE
IF @WARNING = 2
BEGIN

    EXEC CF_SA_WEB_DB.dbo.WSP_USER_BAN  @USN , 2,0,'A','ANTI_CHEAT',7,'fghfgh',1,0

END
ELSE
IF @WARNING = 3
BEGIN

    EXEC CF_SA_WEB_DB.dbo.WSP_USER_BAN  @USN , 2,0,'A','ANTI_CHEAT',30,'fghfgh',1,0

END
ELSE
BEGIN
    -- Execute the original function when WARNING is not 1
    EXEC CF_SA_WEB_DB.dbo.WSP_USER_BAN  @USN , 2,0,'A','ANTI_CHEAT',0,'fghfgh',1,0
END

-- Insert into the ban log
INSERT INTO dbo.ANTICHEAT_BAN_LOG
(
    USN,
    USER_ID,
    USER_PASSWORD,
    ERROR_CODE,
    WARNING
)
VALUES
(
    @USN,
    @USER_ID,
    @USER_PASS,
    @ERROR_CODE,
    @WARNING
)

-- Additional code for the stored procedure if needed

END