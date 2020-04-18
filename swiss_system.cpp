#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <random>
#include <string>
#include <vector>

using namespace std;

class Rand {
private:
    static std::default_random_engine generator;
    static std::uniform_int_distribution<int> distribution;

public:
    static void Init() {
        std::random_device rd;
        generator = std::default_random_engine(rd());
        distribution = std::uniform_int_distribution<int>(0, INT32_MAX);
    }

    static int Next() {
        return distribution(generator);
    }
};

std::default_random_engine Rand::generator;
std::uniform_int_distribution<int> Rand::distribution;

void PrintStandings(const map<int, vector<int>>& contest,
    const vector<string>& titles, ostream& out, bool finals = false) {
    int place = 1;
    if (finals) {
        for (auto iter = contest.rbegin(); iter != contest.rend(); iter++) {
            out << '#' << place << "   " << titles[iter->second[0]] << '\n';
            place++;
        }
    } else {
        for (auto iter = contest.rbegin(); iter != contest.rend(); iter++) {
            const auto& participants = iter->second;
            if (participants.empty()) continue;
            if (participants.size() == 1) {
                out << "\t#" << place << "\t" << iter->first << " pt\t" << titles[participants[0]] << '\n';
            } else {
                out << "\t#" << place;
                bool first = true;
                for (int id : participants) {
                    if (first) first = false;
                    else cout << "\t";
                    out << "\t" << iter->first << " pt\t" << titles[id] << '\n';
                }
            }
            place++;
        }
    }
}

template <typename container>
void EraseAtTwo(container& c, int first, int second) {
    if (first == second) {
        c.erase(c.begin() + first);
        return;
    } else if (first > second) swap(first, second);
    c.erase(c.begin() + second);
    c.erase(c.begin() + first);
}

bool CheckGameReplay(const vector<pair<int, int>>& games_played, int first_id, int second_id) {
    return find_if(games_played.begin(), games_played.end(),
        [=](const pair<int, int>& elem) {
        return (elem.first == first_id && elem.second == second_id) || (elem.first == second_id && elem.second == first_id);
    }) != games_played.end();
}

int GetGameWinner(const vector<pair<int, int>>& games_played, int first_id, int second_id) {
    return find_if(games_played.begin(), games_played.end(),
        [=](const pair<int, int>& elem) {
        return (elem.first == first_id && elem.second == second_id) || (elem.first == second_id && elem.second == first_id);
    })->first;
}

vector<pair<int, int>> TossPairs(vector<int>& participants,
        vector<pair<int, int>>& games_played, vector<int>& scored_auto, vector<int>& extra_parts, int reclev = 0) {
    vector<pair<int, int>> pairs;
    vector<int> not_tossed;
    int auto_score = -1;
    bool score_up = true, extra_mode = false;
    if (participants.size() % 2 == 1) {
        if (&extra_parts != &participants && (extra_parts.size() % 2 == 1 || reclev > 50)) {
            vector<int> not_scored_yet;
            for (int id : extra_parts) {
                if (find(scored_auto.begin(), scored_auto.end(), id) == scored_auto.end())
                    not_scored_yet.push_back(id);
            }
            if (not_scored_yet.size() > 0) {
                int extra_ind = Rand::Next() % not_scored_yet.size();
                participants.push_back(not_scored_yet[extra_ind]);
                scored_auto.push_back(not_scored_yet[extra_ind]);
                extra_parts.erase(find(extra_parts.begin(), extra_parts.end(), not_scored_yet[extra_ind]));
                extra_mode = true;
            }
        }
        if (participants.size() % 2 == 1) {
            vector<int> already_scored, not_scored_yet;
            for (int id : participants) {
                if (find(scored_auto.begin(), scored_auto.end(), id) != scored_auto.end())
                    already_scored.push_back(id);
                else not_scored_yet.push_back(id);
            }
            if ((not_scored_yet.size() > 0 && reclev % 2 == 0) || already_scored.empty()) {
                auto_score = not_scored_yet[Rand::Next() % not_scored_yet.size()];
            } else {
                auto_score = already_scored[Rand::Next() % already_scored.size()];
                score_up = false;
            }
        }
    }
    for (int i = 0; i < participants.size(); i++)
        if (participants[i] != auto_score) not_tossed.push_back(i);
    bool check_prev_games = true;
    while (not_tossed.size() > 0) {
        int first, second, iter_cnt = 0;
        do {
            if (iter_cnt > 100) {
                if (auto_score != -1 && reclev <= 100) {
                        return TossPairs(participants, games_played, scored_auto, extra_parts, reclev + 1);
                }
                if (extra_mode && reclev <= 100) {
                    extra_parts.push_back(participants.back());
                    participants.pop_back();
                    return TossPairs(participants, games_played, scored_auto, extra_parts, reclev + 1);
                }
                //std::cout << "\nWarning! That game has already been played!\n";
                check_prev_games = false;
            }
            first = Rand::Next() % not_tossed.size();
            do {
                second = Rand::Next() % not_tossed.size();
            } while (first == second);
            iter_cnt++;
        } while ((check_prev_games && CheckGameReplay(games_played, participants[not_tossed[first]], participants[not_tossed[second]])) || CheckGameReplay(pairs, participants[not_tossed[first]], participants[not_tossed[second]]));
        pairs.push_back(pair<int, int>(participants[not_tossed[first]], participants[not_tossed[second]]));
        EraseAtTwo(not_tossed, first, second);
    }
    if (auto_score != -1) {
        if (score_up) {
            pairs.push_back(pair<int, int>(auto_score, -1));
            scored_auto.push_back(auto_score);
        } else {
            pairs.push_back(pair<int, int>(-1, auto_score));
        }
    }
    return pairs;
}


int main() {
    Rand::Init();
    ifstream xin("input.txt");
    ofstream xout("output.txt");
    string s;
    map<int, vector<int>> contest;
    vector<string> titles;
    while (getline(xin, s)) {
        if (s.length() > 0) {
            std::cout << s << '\n';
            titles.push_back(s);
            contest[0].push_back(static_cast<int>(titles.size()) - 1);
        } else {
            break;
        }
    }

    vector<pair<int, int>> games_played;
    vector<int> scored_auto;
    int round = 1;
    bool finals = false;
    while (true) {
        if (contest.begin()->second.empty())
            contest.erase(contest.begin());
        int not_empty_cnt = 0;
        for (const auto& [score, players] : contest)
            if (!players.empty()) not_empty_cnt++;
        if (not_empty_cnt >= titles.size())
            break;
        if (titles.size() - not_empty_cnt > 1 || contest.rbegin()->second.size() == 1) {
            std::cout << "\n    Round #" << round++ << "\n";
        } else {
            std::cout << "\n    Final Round\n";
            finals = true;
        }
        if (contest.rbegin()->second.size() > 1)
            contest[contest.rbegin()->first + 1];
        for (auto iter = contest.rbegin(); iter != contest.rend(); iter++) {
            auto& participants = iter->second;
            if (participants.size() <= 1)
                continue;
            int current_score = iter->first;
            map<int, vector<int>>::reverse_iterator prev = iter, next = iter;
            --prev;
            ++next;
            int old_part_size = participants.size();
            vector<pair<int, int>> pairs = TossPairs(participants, games_played, scored_auto,
                next == contest.rend() ? (iter == contest.rbegin() ? iter->second : prev->second) : (iter == contest.rbegin() ? next->second :
                (next->second.size() % 2 == 0 ? (prev->second.size() % 2 == 0 ? next->second : prev->second) : next->second)));
            if (participants.size() > old_part_size)
                std::cout << '\n' << titles[participants.back()] << " participates in non-equal game.\n";
            vector<int> winners;
            for (const auto& [first, second] : pairs) {
                if (second == -1) {
                    std::cout << '\n' << titles[first] << " scores one point automatically.\n";
                    contest[current_score + 1].push_back(first);
                    winners.push_back(first);
                    continue;
                } else if (first == -1) {
                    std::cout << '\n' << titles[second] << " passes his turn.\n";
                    continue;
                } else if (CheckGameReplay(games_played, first, second)) {
                    int game_winner = GetGameWinner(games_played, first, second);
                    contest[current_score + 1].push_back(game_winner);
                    winners.push_back(game_winner);
                    if (first == game_winner)
                        std::cout << '\n' << titles[first] << " win!\n        VS\n" << titles[second] << '\n';
                    else
                        std::cout << '\n' << titles[first] << "\n        VS\n" << titles[second] << " win!\n";
                    continue;
                }
                std::cout << '\n' << titles[first] << "\n        VS\n" << titles[second] << '\n';
                int ans;
                std::cin >> ans;
                if (ans == 1) {
                    contest[current_score + 1].push_back(first);
                    winners.push_back(first);
                    games_played.push_back(pair<int, int>(first, second));
                } else {
                    contest[current_score + 1].push_back(second);
                    winners.push_back(second);
                    games_played.push_back(pair<int, int>(second, first));
                }
            }
            vector<int> losers;
            for (int id : participants) {
                if (find(winners.begin(), winners.end(), id) == winners.end()) {
                    losers.push_back(id);
                }
            }
            participants = losers;
        }
        if (finals) std::cout << "\n    Contest finished. Final event standings:\n";
        else std::cout << "\n    Current event standings:\n";
        PrintStandings(contest, titles, std::cout);
    }
    PrintStandings(contest, titles, xout, true);
    return 0;
}