#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <map>
#include <cmath>

using namespace std;

const int NUM_TEAMS = 10;
const int NUM_ACTIVITIES = 5;
const int NUM_ROUNDS = 5;
const int TEAMS_PER_ACTIVITY = 2;

// Genetic Algorithm parameters (adjusted for smaller problem)
const int POPULATION_SIZE = 1000;
const int NUM_GENERATIONS = 10000;
const double MUTATION_RATE = 0.3;
const double CROSSOVER_RATE = 0.7;
const int ELITE_SIZE = 3;

// Random number generator
random_device rd;
mt19937 gen(rd());

// Schedule representation: schedule[round][activity] = vector of teams
using Schedule = vector<vector<vector<int>>>;

class GeneticScheduler {
private:
    vector<Schedule> population;
    vector<int> fitness;
    
    // Create a random schedule
    Schedule createRandomSchedule() {
        Schedule schedule(NUM_ROUNDS, vector<vector<int>>(NUM_ACTIVITIES));
        
        for (int round = 0; round < NUM_ROUNDS; round++) {
            // Create a permutation of all teams
            vector<int> teams(NUM_TEAMS);
            for (int i = 0; i < NUM_TEAMS; i++) teams[i] = i;
            shuffle(teams.begin(), teams.end(), gen);
            
            // Distribute teams across activities
            int teamIdx = 0;
            for (int activity = 0; activity < NUM_ACTIVITIES; activity++) {
                for (int slot = 0; slot < TEAMS_PER_ACTIVITY; slot++) {
                    schedule[round][activity].push_back(teams[teamIdx++]);
                }
            }
        }
        
        return schedule;
    }
    
    // Calculate fitness (lower is better, 0 is perfect)
    int calculateFitness(const Schedule& schedule) {
        int violations = 0;
        
        // Constraint 1: Each team does each activity exactly once
        vector<vector<int>> teamActivityCount(NUM_TEAMS, vector<int>(NUM_ACTIVITIES, 0));
        
        for (int round = 0; round < NUM_ROUNDS; round++) {
            for (int activity = 0; activity < NUM_ACTIVITIES; activity++) {
                for (int team : schedule[round][activity]) {
                    teamActivityCount[team][activity]++;
                }
            }
        }
        
        for (int team = 0; team < NUM_TEAMS; team++) {
            for (int activity = 0; activity < NUM_ACTIVITIES; activity++) {
                violations += abs(teamActivityCount[team][activity] - 1);
            }
        }
        
        // Constraint 2: No two teams meet more than once
        map<pair<int, int>, int> pairMeetings;
        
        for (int round = 0; round < NUM_ROUNDS; round++) {
            for (int activity = 0; activity < NUM_ACTIVITIES; activity++) {
                const vector<int>& teams = schedule[round][activity];
                // Check all pairs at this activity
                for (size_t i = 0; i < teams.size(); i++) {
                    for (size_t j = i + 1; j < teams.size(); j++) {
                        int t1 = min(teams[i], teams[j]);
                        int t2 = max(teams[i], teams[j]);
                        pairMeetings[{t1, t2}]++;
                    }
                }
            }
        }
        
        for (const auto& [pair, count] : pairMeetings) {
            if (count > 1) {
                violations += (count - 1) * 2; // Weight pair violations more heavily
            }
        }
        
        return violations;
    }
    
    // Tournament selection
    int tournamentSelect() {
        uniform_int_distribution<> dis(0, POPULATION_SIZE - 1);
        int idx1 = dis(gen);
        int idx2 = dis(gen);
        int idx3 = dis(gen);
        
        int best = idx1;
        if (fitness[idx2] < fitness[best]) best = idx2;
        if (fitness[idx3] < fitness[best]) best = idx3;
        
        return best;
    }
    
    // Crossover: combine rounds from two parents
    Schedule crossover(const Schedule& parent1, const Schedule& parent2) {
        uniform_real_distribution<> prob(0.0, 1.0);
        
        if (prob(gen) > CROSSOVER_RATE) {
            return (prob(gen) < 0.5) ? parent1 : parent2;
        }
        
        Schedule child = parent1;
        uniform_int_distribution<> roundDis(0, NUM_ROUNDS - 1);
        
        // Uniform crossover: randomly select rounds from each parent
        for (int round = 0; round < NUM_ROUNDS; round++) {
            if (prob(gen) < 0.5) {
                child[round] = parent2[round];
            }
        }
        
        return child;
    }
    
    // Multiple mutation strategies
    void mutate(Schedule& schedule) {
        uniform_real_distribution<> prob(0.0, 1.0);
        uniform_int_distribution<> roundDis(0, NUM_ROUNDS - 1);
        uniform_int_distribution<> activityDis(0, NUM_ACTIVITIES - 1);
        uniform_int_distribution<> slotDis(0, TEAMS_PER_ACTIVITY - 1);
        
        // Apply multiple mutations
        int numMutations = (prob(gen) < MUTATION_RATE) ? 1 + (prob(gen) < 0.3 ? 1 : 0) : 0;
        
        for (int m = 0; m < numMutations; m++) {
            int mutationType = uniform_int_distribution<>(0, 3)(gen);
            
            if (mutationType == 0) {
                // Swap two teams within the same round
                int round = roundDis(gen);
                int act1 = activityDis(gen);
                int slot1 = slotDis(gen);
                int act2 = activityDis(gen);
                int slot2 = slotDis(gen);
                swap(schedule[round][act1][slot1], schedule[round][act2][slot2]);
                
            } else if (mutationType == 1) {
                // Swap two teams across different rounds (same positions)
                int round1 = roundDis(gen);
                int round2 = roundDis(gen);
                if (round1 != round2) {
                    int activity = activityDis(gen);
                    int slot = slotDis(gen);
                    swap(schedule[round1][activity][slot], schedule[round2][activity][slot]);
                }
                
            } else if (mutationType == 2) {
                // Shuffle teams within an activity in a round
                int round = roundDis(gen);
                int activity = activityDis(gen);
                shuffle(schedule[round][activity].begin(), schedule[round][activity].end(), gen);
                
            } else {
                // Swap entire activities between two rounds
                int round1 = roundDis(gen);
                int round2 = roundDis(gen);
                if (round1 != round2) {
                    int activity = activityDis(gen);
                    swap(schedule[round1][activity], schedule[round2][activity]);
                }
            }
        }
    }
    
public:
    Schedule solve() {
        cout << "Initializing population of " << POPULATION_SIZE << " schedules..." << endl;
        
        // Initialize population
        for (int i = 0; i < POPULATION_SIZE; i++) {
            population.push_back(createRandomSchedule());
            fitness.push_back(calculateFitness(population[i]));
        }
        
        int bestIdx = 0;
        for (int i = 1; i < POPULATION_SIZE; i++) {
            if (fitness[i] < fitness[bestIdx]) bestIdx = i;
        }
        
        cout << "Initial best fitness: " << fitness[bestIdx] << endl;
        cout << "Starting evolution...\n" << endl;
        
        int generationsWithoutImprovement = 0;
        int lastBestFitness = fitness[bestIdx];
        
        for (int generation = 0; generation < NUM_GENERATIONS; generation++) {
            vector<Schedule> newPopulation;
            vector<int> newFitness;
            
            // Elitism: keep the best solutions
            vector<pair<int, int>> fitnessIndices;
            for (int i = 0; i < POPULATION_SIZE; i++) {
                fitnessIndices.push_back({fitness[i], i});
            }
            sort(fitnessIndices.begin(), fitnessIndices.end());
            
            for (int i = 0; i < ELITE_SIZE && i < POPULATION_SIZE; i++) {
                int idx = fitnessIndices[i].second;
                newPopulation.push_back(population[idx]);
                newFitness.push_back(fitness[idx]);
            }
            
            // Generate rest of population
            while (newPopulation.size() < POPULATION_SIZE) {
                int parent1Idx = tournamentSelect();
                int parent2Idx = tournamentSelect();
                
                Schedule child = crossover(population[parent1Idx], population[parent2Idx]);
                mutate(child);
                
                int childFitness = calculateFitness(child);
                newPopulation.push_back(child);
                newFitness.push_back(childFitness);
            }
            
            population = newPopulation;
            fitness = newFitness;
            
            // Find best in current generation
            bestIdx = 0;
            for (int i = 1; i < POPULATION_SIZE; i++) {
                if (fitness[i] < fitness[bestIdx]) bestIdx = i;
            }
            
            // Progress reporting
            if (generation % 100 == 0 || fitness[bestIdx] < lastBestFitness) {
                cout << "Generation " << generation << " - Best fitness: " << fitness[bestIdx] 
                     << " (Violations: " << fitness[bestIdx] << ")" << endl;
            }
            
            // Track improvement
            if (fitness[bestIdx] < lastBestFitness) {
                generationsWithoutImprovement = 0;
                lastBestFitness = fitness[bestIdx];
            } else {
                generationsWithoutImprovement++;
            }
            
            // Early termination if perfect solution found
            if (fitness[bestIdx] == 0) {
                cout << "\n*** PERFECT SOLUTION FOUND at generation " << generation << "! ***" << endl;
                break;
            }
            
            // Restart if stuck
            if (generationsWithoutImprovement > 2000) {
                cout << "  (Stuck - injecting diversity...)" << endl;
                for (int i = ELITE_SIZE; i < POPULATION_SIZE / 2; i++) {
                    population[i] = createRandomSchedule();
                    fitness[i] = calculateFitness(population[i]);
                }
                generationsWithoutImprovement = 0;
            }
        }
        
        cout << "\nEvolution complete!" << endl;
        cout << "Final best fitness: " << fitness[bestIdx] << endl;
        return population[bestIdx];
    }
    
    void printSchedule(const Schedule& schedule) {
        cout << "==== SCHEDULE ====" << endl;
        
        for (int round = 0; round < NUM_ROUNDS; round++) {
            cout << "\n--- Round " << (round + 1) << " ---" << endl;
            for (int activity = 0; activity < NUM_ACTIVITIES; activity++) {
                cout << "  Activity " << (activity + 1) << ": ";
                for (size_t i = 0; i < schedule[round][activity].size(); i++) {
                    cout << "T" << (schedule[round][activity][i] + 1);
                    if (i < schedule[round][activity].size() - 1) cout << ", ";
                }
                cout << endl;
            }
        }
    }
    
    void validateSchedule(const Schedule& schedule) {
        cout << "==== VALIDATION REPORT ====" << endl;
        
        bool overallValid = true;
        
        // Check: Each team does each activity exactly once
        cout << "\n[1] Checking: Each team does each activity exactly once..." << endl;
        bool check1Valid = true;
        int check1Errors = 0;
        
        for (int team = 0; team < NUM_TEAMS; team++) {
            vector<int> activityCount(NUM_ACTIVITIES, 0);
            for (int round = 0; round < NUM_ROUNDS; round++) {
                for (int activity = 0; activity < NUM_ACTIVITIES; activity++) {
                    for (int t : schedule[round][activity]) {
                        if (t == team) activityCount[activity]++;
                    }
                }
            }
            
            for (int activity = 0; activity < NUM_ACTIVITIES; activity++) {
                if (activityCount[activity] != 1) {
                    if (check1Errors < 5) {
                        cout << "    ERROR: Team " << (team + 1) << " does Activity " 
                             << (activity + 1) << " " << activityCount[activity] << " time(s)" << endl;
                    }
                    check1Errors++;
                    check1Valid = false;
                }
            }
        }
        
        if (check1Errors > 5) {
            cout << "    ... and " << (check1Errors - 5) << " more errors" << endl;
        }
        
        if (check1Valid) {
            cout << "    PASS - All teams do each activity exactly once" << endl;
        } else {
            cout << "    FAIL - " << check1Errors << " constraint violations" << endl;
            overallValid = false;
        }
        
        // Check: No two teams meet more than once
        cout << "\n[2] Checking: No two teams meet at the same activity more than once..." << endl;
        map<pair<int, int>, vector<pair<int, int>>> pairMeetings;
        
        for (int round = 0; round < NUM_ROUNDS; round++) {
            for (int activity = 0; activity < NUM_ACTIVITIES; activity++) {
                const vector<int>& teams = schedule[round][activity];
                for (size_t i = 0; i < teams.size(); i++) {
                    for (size_t j = i + 1; j < teams.size(); j++) {
                        int t1 = min(teams[i], teams[j]);
                        int t2 = max(teams[i], teams[j]);
                        pairMeetings[{t1, t2}].push_back({round + 1, activity + 1});
                    }
                }
            }
        }
        
        int check2Errors = 0;
        for (const auto& [pair, meetings] : pairMeetings) {
            if (meetings.size() > 1) {
                if (check2Errors < 5) {
                    cout << "    ERROR: Teams " << (pair.first + 1) << " and " << (pair.second + 1) 
                         << " meet " << meetings.size() << " times:";
                    for (const auto& [r, a] : meetings) {
                        cout << " (R" << r << ",A" << a << ")";
                    }
                    cout << endl;
                }
                check2Errors++;
            }
        }
        
        if (check2Errors > 5) {
            cout << "    ... and " << (check2Errors - 5) << " more team pair violations" << endl;
        }
        
        if (check2Errors == 0) {
            cout << "    PASS - No team pairs meet more than once" << endl;
        } else {
            cout << "    FAIL - " << check2Errors << " team pairs meet multiple times" << endl;
            overallValid = false;
        }
        
        // Summary
        if (overallValid) {
            cout << "STATUS: VALID SCHEDULE" << endl;
        } else {
            cout << "STATUS: INVALID SCHEDULE - " << (check1Errors + check2Errors) 
                 << " total violations" << endl;
        }
    }
};

int main() {
    cout << "  TEAM SCHEDULING - TEST VERSION" << endl;
    cout << "\nConfiguration:" << endl;
    cout << "  Teams:              " << NUM_TEAMS << endl;
    cout << "  Activities:         " << NUM_ACTIVITIES << endl;
    cout << "  Rounds:             " << NUM_ROUNDS << endl;
    cout << "  Teams per Activity: " << TEAMS_PER_ACTIVITY << endl;
    cout << "\nGA Parameters:" << endl;
    cout << "  Population Size:    " << POPULATION_SIZE << endl;
    cout << "  Max Generations:    " << NUM_GENERATIONS << endl;
    cout << "  Mutation Rate:      " << MUTATION_RATE << endl;
    cout << "  Crossover Rate:     " << CROSSOVER_RATE << endl;
    cout << "  Elite Size:         " << ELITE_SIZE << endl;
    
    GeneticScheduler scheduler;
    Schedule solution = scheduler.solve();
    
    scheduler.printSchedule(solution);
    scheduler.validateSchedule(solution);
    
    return 0;
}