-- ==========================================================
-- Vortex Anti-Cheat Integration
-- Target Procedure: [dbo].[CF_AUTH]
-- Injection Point: Executed immediately after successful user authentication.
-- Purpose: Purges stale/spoofed AC sessions to enforce a fresh heartbeat.
-- ==========================================================

IF @p_Result > 0
BEGIN
    -- [Vortex] Wipe old sessions to prevent session hijacking
    DELETE FROM [dbo].[CF_ANTICHEAT].ANTICHEAT_AUTH WHERE User_id = @p_User_id;
    
    -- (The rest of the game's reward logic continues here...)
END




-- ==========================================================
-- Vortex Anti-Cheat Integration
-- Target Procedure: [dbo].[SP_GS_GAME_LOGIN]
-- Injection Point: Before resolving final player stats and allowing server join.
-- Purpose: Smart polling for AC session heartbeat with automatic abuse logging.
-- ==========================================================

DECLARE @usn_check INT = @p_usn + 22;
DECLARE @EndTime DATETIME = DATEADD(SECOND, 10, GETDATE());
SET @p_result = -1;

-- [Vortex] 10-Second Smart Polling: Wait for AC Client to establish session
WHILE GETDATE() < @EndTime
BEGIN
    IF EXISTS (SELECT 1 FROM [CF_ANTICHEAT].[dbo].ANTICHEAT_AUTH WHERE Usn = @usn_check) OR (@p_authority != 'N')
    BEGIN
        SET @p_result = 1; -- Valid Session Established
        -- DELETE FROM ANTICHEAT_AUTH WHERE Usn = @usn_check; -- (Optional: strict one-time use)
        BREAK; 
    END

    WAITFOR DELAY '00:00:01';
END

-- [Vortex] Abuse Logging: If no AC session is found after 10 seconds
IF @p_result = -1
BEGIN
    -- Auto-create Audit Table if not exists
    IF OBJECT_ID('[CF_ANTICHEAT].[dbo].[CF_ANTICHEAT_ABUSE_LOG]', 'U') IS NULL
    BEGIN
        CREATE TABLE [CF_ANTICHEAT].[dbo].[CF_ANTICHEAT_ABUSE_LOG] (
            [LogID] [int] IDENTITY(1,1) NOT NULL PRIMARY KEY CLUSTERED,
            [USN] [int] NOT NULL,
            [USER_ID] [varchar](50) NULL,
            [Reason] [nvarchar](255) NULL,
            [LogTime] [datetime] DEFAULT GETDATE()
        );
    END

    -- Fetch UserID for accurate logging
    DECLARE @FetchedUserID VARCHAR(50);
    SELECT TOP 1 @FetchedUserID = USER_ID 
    FROM [CF_SA_GAME].[dbo].[CF_MEMBER] 
    WHERE USN = @p_usn;

    -- Log the bypass attempt
    INSERT INTO [CF_ANTICHEAT].[dbo].[CF_ANTICHEAT_ABUSE_LOG] ([USN], [USER_ID], [Reason], [LogTime]) 
    VALUES (@p_usn, ISNULL(@FetchedUserID, 'Unknown'), 'Login attempted without Anti-Cheat', GETDATE());

    PRINT 'VORTEX: Failed - Bypass attempt logged successfully.';
END