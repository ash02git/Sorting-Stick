#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <thread>

namespace Gameplay
{
    namespace Collection
    {
        class StickCollectionView;
        class StickCollectionModel;
        struct Stick;
        enum class SortType;
        enum class SortState;

        class StickCollectionController
        {
        private:
            StickCollectionView* collection_view;
            StickCollectionModel* collection_model;

            std::vector<Stick*> sticks;
            SortType sort_type;

            std::thread sort_thread;

            int number_of_comparisons;
            int number_of_array_access;
            int current_operation_delay;

            int delay_in_ms;
            sf::String time_complexity;

            SortState sort_state;

            int color_delay;

            void initializeSticks();
            float calculateStickWidth();
            float calculateStickHeight(int array_pos);

            void updateStickPosition();
            void updateStickPosition(int i);
            void shuffleSticks();
            bool compareSticksByData(const Stick* a, const Stick* b) const;

            void resetSticksColor();
            void resetVariables();

            void processSortThreadState();

            bool isCollectionSorted();
            void destroy();

            void setCompletedColor();

            void processBubbleSort();
            void processInsertionSort();
            void processSelectionSort();
            void processInPlaceMergeSort();
            void processMergeSort();
            void processQuickSort();
            void processRadixSort();

            void inPlaceMerge(int left, int mid, int right);
            void inPlaceMergeSort(int left,int right);

            void outPlaceMergeSort(int left, int right);
            void outPlaceMerge(int left, int mid, int right);

            void quickSort(int low, int high);
            int partition(int low, int high);

            void radixSort();
            void countSort(int exponent);

        public:
            StickCollectionController();
            ~StickCollectionController();

            void initialize();
            void update();
            void render();

            void reset();
            void sortElements(SortType sort_type);

            SortType getSortType();
            int getNumberOfComparisons();
            int getNumberOfArrayAccess();

            int getNumberOfSticks();
            int getDelayMilliseconds();
            sf::String getTimeComplexity();
        };
    }
}