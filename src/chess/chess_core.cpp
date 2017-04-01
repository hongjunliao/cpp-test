/*!
 * This file is PART of chess project
 * gobang, FIR, you can call it
 * @author hongjun.liao <docici@126.com>
 * @date 2017/03/31

 * 1.rules for chess/gobang:
 * for any chess_point(x, y) in chess_point[CHESS_X][CHESS_Y], if one of
 * (1)number of chess_point(x + n, y + n).hit >= 5
 * (2)number of chess_point(x, y + n).hit >= 5
 * (2)number of chess_point(x + n, y).hit >= 5
 * (2)number of chess_point(x - n, y + n).hit >= 5
 * found, then win
 */

#include "chess.h"
#include <stdio.h>
#include <string.h> /* memcpy */

/* chess_test.cpp */
extern int verbose;

static int chess_board_find_first(chess_board const& board, chess_pt const& pt, chess_pt ** pts, int  n);

static int chess_board_find_first(chess_board const& board, chess_pt const& pt, chess_pt ** pts, int  n)
{
	if(verbose)
		fprintf(stdout, "%s: begin, [%d,%d,%d]\n", __FUNCTION__, pt.x, pt.y, pt.hit);
	bool l_off = false, r_off = false;
	bool t_off = false, b_off = false;
	bool lt_off = false, rb_off = false;
	bool lb_off = false, rt_off = false;

	int n_lr = 0, n_tb = 0, n_ltrb = 0, n_lbrt = 0;
	chess_pt * pts_lr[n], * pts_tb[n], * pts_ltrb[n], * pts_lbrt[n];

	for(int x1 = pt.x + 1, x2 = pt.x - 1, y1 = pt.y + 1, y2 = pt.y - 1;
			x1 < board.W || y1 < board.H || x2 > -1 || y2 > -1;
			++x1, ++y1, --x2, --y2)
	{
		if(verbose)
			fprintf(stdout, "%s: x1=%d, x2=%d, y1=%d, y2=%d, l_off=%d, r_off=%d\n", __FUNCTION__, x1, x2, y1, y2, l_off, r_off);
		/* find left-right */
		if(!l_off || !r_off){
			if(verbose)
				fprintf(stdout, "%s: search left-right\n", __FUNCTION__);
			if(!l_off && x2 > -1){
				auto & p = board.pts[pt.y * board.W + x2];
				if(verbose)
					fprintf(stdout, "%s:left [%d,%d,%d], ", __FUNCTION__, p.x, p.y, p.hit);
				if(p.hit != pt.hit){
					l_off = true;
					if(verbose)
						fprintf(stdout, "off\n");
				}
				else{
					pts_lr[n_lr++] = &p;
					if(verbose)
						fprintf(stdout, "n=%d\n", n_lr);
				}
			}
			if(!r_off && x1 < board.W){
				auto & p = board.pts[pt.y * board.W + x1];
				if(verbose)
					fprintf(stdout, "%s:right [%d,%d,%d], ", __FUNCTION__, p.x, p.y, p.hit);
				if(p.hit != pt.hit){
					r_off = true;
					if(verbose)
						fprintf(stdout, "off\n");
				}
				else{
					pts_lr[n_lr++] = &p;
					if(verbose)
						fprintf(stdout, "n=%d\n", n_lr);
				}
			}

			if(n_lr >= n){
				memcpy(pts, pts_lr, sizeof(chess_pt *) * n);
				return 0;
			}
		}
//
//		/* find top-bottom */
		if(!t_off || !b_off){
			if(!t_off && y2 > -1){
				auto & p = board.pts[y2 * board.W + pt.x];
				if(p.hit != pt.hit)
					t_off = true;
				else
					pts_tb[n_tb++] = &p;
			}

			if(!b_off && y1 < board.H){
				auto & p = board.pts[y1 * board.W + pt.x];
				if(p.hit != pt.hit)
					b_off = true;
				else
					pts_tb[n_tb++] = &p;
			}
			if(n_tb >= n){
				memcpy(pts, pts_tb, sizeof(chess_pt *) * n);
				return 0;
			}
		}
//
//		/* find left-top-right_bottom */
		if(!lt_off || !rb_off){
			if(!lt_off && x2 > -1 && y2 > -1){
				auto & p = board.pts[y2 * board.W + x2];
				if(p.hit != pt.hit)
					lt_off = true;
				else
					pts_ltrb[n_ltrb++] = &p;
			}
			if(!rb_off && x1 < board.W && y1 < board.H){
				auto & p = board.pts[y1 * board.W + x1];
				if(p.hit != pt.hit)
					rb_off = true;
				else
					pts_ltrb[n_ltrb++] = &p;
			}
			if(n_ltrb >= n){
				memcpy(pts, pts_ltrb, sizeof(chess_pt *) * n);
				return 0;
			}
		}
//
//		/* find left_bottom-right-top */
		if(!lb_off || !rt_off){
			if(!lb_off && y1 < board.H && x2 > -1){
				auto & p = board.pts[y1 * board.W + x2];
				if(p.hit != pt.hit)
					lb_off = true;
				else
					pts_lbrt[n_lbrt++] = &p;
			}

			if(!rt_off && y2 > -1 && x1 < board.W){
				auto & p = board.pts[y2 * board.W + x1];
				if(p.hit != pt.hit)
					rt_off = true;
				else
					pts_lbrt[n_lbrt++] = &p;
			}
			if(n_lbrt >= n){
				memcpy(pts, pts_lbrt, sizeof(chess_pt *) * n);
				return 0;
			}
		}
	}
	if(verbose)
		fprintf(stdout, "%s: end\n", __FUNCTION__);
	return -1;
}

void chess_board_reset(chess_board & board)
{
	for(int i = 0; i != board.H; ++i){
		for(int j = i * board.W; j != (i + 1) * board.W; ++j){
			board.pts[j].x = j % board.W;
			board.pts[j].y = i;
			board.pts[j].hit = 0;
			board.pts[j].win = 0;
		}
	}
}

int chess_board_find(chess_board const& board, chess_pt ** pts, int n)
{
	for(int i = 0; i != board.H; ++i){
		for(int j = i * board.W; j != (i + 1) * board.W; ++j){

			if(board.pts[j].hit == 0)
				continue;

			/* board.pts[j] already hit, find n -  1 only */
			auto result = chess_board_find_first(board, board.pts[j], pts, n - 1);

			if(result == 0){

				board.pts[j].win = 1;
				for(int i = 0; i < n - 1; ++i){
					pts[i]->win = 1;
				}

				return 0;
			}
		}
	}
	return -1;
}
