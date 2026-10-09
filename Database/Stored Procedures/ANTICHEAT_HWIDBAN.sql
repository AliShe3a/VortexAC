ALTER PROCEDURE [dbo].[ANTICHEAT_HWIDBAN]
  @Reg1 AS varchar(1024) ,
  @Reg2 AS varchar(1024) ,
  @User_name AS varchar (256),
  @Domain_name AS varchar(256) ,@Client_IP AS varchar(256)
	
AS

BEGIN

 

 -- IF (DATEDIFF(ss, @Time, GETUTCDATE()) < 10  AND DATEDIFF(ss, @Time, GETUTCDATE()) >= 0 OR DATEDIFF(ss, @Time, GETUTCDATE()) >=-15 )
	--BEGIN 
	
 
	 BEGIN
	-- if( SIGN(DATEDIFF(ss, @Time, GETUTCDATE())) = 1 )
	-- 	 BEGIN

 INSERT INTO [dbo].[ANTICHEAT_HARDWARE_BAN]
          (                    
            REG1                     ,
            REG2                  ,
            USERNAME                      ,
						DOMAIN_NAME ,
						CLIENT_IP
          ) 
     VALUES 
          ( 
            @Reg1,
            @Reg2,
            @User_name
						,@Domain_name
						,@Client_IP
          ) 
				--	END
				
								END
						
															
--END

	--ELSE
												--					-		INSERT INTO dbo.ANTICHEAT_MISMATCH
        --  (                    
         --   Usn                     ,
        --    User_id                  ,
         --   User_pass                      ,
         --   User_name                 ,
					--	Domain_name
					--	,Registry1,Registry2,Time_MISMATCH,Client_IP
        --  ) 
   --  VALUES 
        --  ( 
         --   @Usn,
          --  @User_ID,
          --  @User_Pass,
          --  @User_name
				--		,@Domain_name
					--	,@Reg1
					--	,@Reg2,DATEDIFF(ss, @Time, GETUTCDATE()),@Client_IP
         -- ) 
END