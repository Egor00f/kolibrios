@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm img_transform.asm img_transform.kex
@kpack img_transform.kex
@erase lang.inc
@pause
