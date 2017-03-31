/*!
 * This file is PART of chess project
 * gobang, FIR, you can call it
 * @author hongjun.liao <docici@126.com>
 * @date 2017/03/31
 */
#ifndef DOCICI_CHESS_H_
#define DOCICI_CHESS_H_

struct chess_pt {
	int x, y;		/* pos */
	int hit;		/* 0: not hit, 1: black, 2: white */
};

struct chess_board {
	chess_pt * pts; /* count of W * H */
	int W, H;		/* width, height */
};

/* reset to an empty chess board */
void chess_board_reset(chess_board & board);

/* find win
 * @param pts:  out, if found, return these chess_pts
 * @param n:    in, usually 5
 * @return 0 on success, found! */
int chess_board_find(chess_board const& board, chess_pt ** pts, int n);

#endif /* DOCICI_CHESS_H_ */
