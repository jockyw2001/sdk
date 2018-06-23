.PHONY : all clean
all:
	$(DB_MAKE) -C ./console/ all
	$(DB_MAKE) -C ./service/ all
	$(DB_MAKE) -C ./onebin/ all
	$(DB_MAKE) -C ./exec/ all

clean:
	$(DB_MAKE) -C ./exec/ clean
	$(DB_MAKE) -C ./onebin/ clean
	$(DB_MAKE) -C ./console/ clean
	$(DB_MAKE) -C ./service/ clean