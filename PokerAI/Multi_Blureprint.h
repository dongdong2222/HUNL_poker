// ################################################################################
// #
// #   Copyright 2022 The DecisionHoldem Authors，namely，Qibin Zhou，
// #   Dongdong Bai，Junge Zhang and Kaiqi Huang. All Rights Reserved.
// #
// #   Licensed under the GNU AFFERO GENERAL PUBLIC LICENSE
// #                 Version 3, 19 November 2007
// #
// #   This program is distributed in the hope that it will be useful,
// #   but WITHOUT ANY WARRANTY; without even the implied warranty of
// #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// #   GNU Affero General Public License for more details.
// #
// #   You should have received a copy of the GNU Affero General Public License 
// #   along with this program.  If not, see <http://www.gnu.org/licenses/>.
// #
// ################################################################################
#pragma once
#include "BlueprintMCCFR.h"
#include "util/ThreadPool.h"
#include "tree/Exploitability.h"
using namespace std;

const int threadnum = 100;
const int multistrategy_interval = 5000, multin_iterations = 100000000, multilcfr_threshold = 4000000, multidiscount_interval = 10000;
const int multiprune_threshold = 100000, multic = -200000000, multin_players = 2, multidump_iteration = 100000, multiupdate_threshold = 1000;
const int each_thread_iters = 2000;
strategy_node* root = new strategy_node();

void each_thread(int t) {

	struct timeval start, end;
	gettimeofday(&start, NULL);
	mt19937_64 _rng_gen(rand());
	Player players[] = { Player(20000),Player(20000) };
	PokerTable table(2, players);
	strategy_node* pref[2];

	Pokerstate state(table);
	for (int k = 1; k <= each_thread_iters; k++) {
		state.reset_game_single(); // dongju : dealing private cards, community cards
		for (int i = 0; i < n_players; i++) {
			state.reset_game(); // dongju : reset game with same private cards and community cards
			//clusters[0] = preflop(0 ~ 168), clusters[1] = flop(0 ~ 49999), clusters[2] = turn(0 ~ 4999), clusters[3] = river(0 ~ 999)
			pref[0] = root->actions + state.table.players[0].clusters[0]; // player0가 뽑은 카드?
			pref[1] = root->actions + state.table.players[1].clusters[0]; // player1가 뽑은 카드?
			if (t > multiprune_threshold) {
				int dr = 1;//rand() % 100;
				if (dr < 5)
					blueprint_cfr(pref, state, i, 1);
				else
					blueprint_cfrp(pref, state, i, multic, 1);
			}
			else
				blueprint_cfr(pref, state, i, 1);
		}
	}
	delete state.table.deck.cards;
	for (int i = 0; i < state.table.playerlen; i++)
		delete state.table.players[i].clusters;
	gettimeofday(&end, NULL);
	cout << "iter :" << t << ",each thread time:" << ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)) / 1000000.0 << endl;

}
void multiprocess_blueprint() {			//program exit //dongju: training

	{
		Player players[] = { Player(20000),Player(20000) };
		PokerTable table(2, players);
		Pokerstate state(table);
		state.reset_game();
		custom_build_preflop(root, state);
		// bulid_preflop(root, state);
		// visualization(root, "visualize/init_gameTree.stgy");

		state.reset_game();
		check_subgame(root, state);
		//state.reset_game();
		//cout << getcfv_whole_holdem(root, state, 0) << endl;
		//state.reset_game();
		//cout << getcfv_whole_holdem(root, state, 1) << endl;
		delete[] players[0].clusters;
		delete[] players[1].clusters;
		delete[] table.deck.cards;
	}
	
	std::condition_variable dumpwait;
	mutex mtx;
	threadpool pool(threadnum, &dumpwait);
	srand(time(0));
	for (int t = 1; t <= multin_iterations; t++) {
		std::unique_lock<mutex> lck(mtx); //dongju : 생성과 동시에 mtx로 lock됨
		pool.commit(each_thread, t);
		if (t % multistrategy_interval == 0) {
			dumpwait.wait(lck);
			while (pool.acttaskNum.load() != 0)
				sleep(1);
			cout << "iter :" << t << endl;
			if (t <= multilcfr_threshold && t % multidiscount_interval == 0) {
				double d = ((double)t / multidiscount_interval) / (((double)t / multidiscount_interval) + 1);
				dfs_discount(root, d, true);
			}
			else
				update_strategy(root, true);
			if (t % multidump_iteration == 0) {
				dump(root, "cluster/multi/blueprint_strategy.dat");
			}
		}
	}
}
