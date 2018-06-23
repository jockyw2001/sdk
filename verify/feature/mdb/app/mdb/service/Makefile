all:
	$(DB_MAKE) -f ./MakeObj.mk all
	$(DB_MAKE) -f ./MakeLib.mk all
	$(DB_MAKE) -f ./MakeApp.mk all
clean:
	$(DB_MAKE) -f ./MakeApp.mk clean
	$(DB_MAKE) -f ./MakeLib.mk clean
	$(DB_MAKE) -f ./MakeObj.mk clean