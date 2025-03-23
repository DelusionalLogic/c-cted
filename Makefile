SRC_DIR = src
CMD_DIR = cmd
TST_DIR = test

OBJ_DIR = obj

CMD_SRC = $(wildcard $(CMD_DIR)/*.c)
CMD_OBJ = $(CMD_SRC:%=$(OBJ_DIR)/%.o)

TST_SRC = $(wildcard $(TST_DIR)/*.c)
TST_RUN = $(TST_SRC:$(TST_DIR)/%.c=$(OBJ_DIR)/test_runner/%)

CC_SRC = $(wildcard $(SRC_DIR)/*.c)
CC_OBJ = $(CC_SRC:%=$(OBJ_DIR)/%.o)

CFLAGS = -Wall -I$(SRC_DIR)
CFLAGS += -g
LDFLAGS =

$(OBJ_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

app: $(CC_OBJ) $(CMD_OBJ)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CC_OBJ) $(CMD_OBJ) -o $@

$(OBJ_DIR)/test_runner/%: $(OBJ_DIR)/$(TST_DIR)/%.c.o $(CC_OBJ)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(CC_OBJ) $< -o $@

.PHONY: test
test: $(TST_RUN)
	$(foreach var,$(TST_RUN),./$(var) &&) true

.PHONY: all
all: app $(TST_RUN)

.PHONY: clean
clean:
	@$(RM) -rf obj/
