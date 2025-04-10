#include "Gameplay/Collection/StickCollectionController.h"
#include "Gameplay/Collection/StickCollectionView.h"
#include "Gameplay/Collection/StickCollectionModel.h"
#include "Gameplay/GameplayService.h"
#include "Global/ServiceLocator.h"
#include "Gameplay/Collection/Stick.h"
#include <random>
#include <iostream>

namespace Gameplay
{
	namespace Collection
	{
		using namespace UI::UIElement;
		using namespace Global;
		using namespace Graphics;

		StickCollectionController::StickCollectionController()
		{
			collection_view = new StickCollectionView();
			collection_model = new StickCollectionModel();

			for (int i = 0; i < collection_model->number_of_elements; i++) sticks.push_back(new Stick(i));
		}

		StickCollectionController::~StickCollectionController()
		{
			destroy();
		}

		void StickCollectionController::initialize()
		{
			sort_state = SortState::NOT_SORTING;
			collection_view->initialize(this);
			initializeSticks();
			reset();
		}

		void StickCollectionController::initializeSticks()
		{
			float rectangle_width = calculateStickWidth();


			for (int i = 0; i < collection_model->number_of_elements; i++)
			{
				float rectangle_height = calculateStickHeight(i); //calc height

				sf::Vector2f rectangle_size = sf::Vector2f(rectangle_width, rectangle_height);

				sticks[i]->stick_view->initialize(rectangle_size, sf::Vector2f(0, 0), 0, collection_model->element_color);
			}
		}

		void StickCollectionController::update()
		{
			processSortThreadState();
			collection_view->update();
			for (int i = 0; i < sticks.size(); i++) sticks[i]->stick_view->update();
		}

		void StickCollectionController::render()
		{
			collection_view->render();
			for (int i = 0; i < sticks.size(); i++) sticks[i]->stick_view->render();
		}

		float StickCollectionController::calculateStickWidth()
		{
			float total_space = static_cast<float>(ServiceLocator::getInstance()->getGraphicService()->getGameWindow()->getSize().x);

			// Calculate total spacing as 10% of the total space
			float total_spacing = collection_model->space_percentage * total_space;

			// Calculate the space between each stick
			float space_between = total_spacing / (collection_model->number_of_elements - 1);
			collection_model->setElementSpacing(space_between);

			// Calculate the remaining space for the rectangles
			float remaining_space = total_space - total_spacing;

			// Calculate the width of each rectangle
			float rectangle_width = remaining_space / collection_model->number_of_elements;

			return rectangle_width;
		}

		float StickCollectionController::calculateStickHeight(int array_pos)
		{
			return (static_cast<float>(array_pos + 1) / collection_model->number_of_elements) * collection_model->max_element_height;
		}

		void StickCollectionController::updateStickPosition()
		{
			for (int i = 0; i < sticks.size(); i++)
			{
				float x_position = (i * sticks[i]->stick_view->getSize().x) + ((i + 1) * collection_model->elements_spacing);
				float y_position = collection_model->element_y_position - sticks[i]->stick_view->getSize().y;

				sticks[i]->stick_view->setPosition(sf::Vector2f(x_position, y_position));
			}
		}

		void StickCollectionController::updateStickPosition(int i)
		{
			float x_position = (i * sticks[i]->stick_view->getSize().x) + ((i)*collection_model->elements_spacing);
			float y_position = collection_model->element_y_position - sticks[i]->stick_view->getSize().y;

			sticks[i]->stick_view->setPosition(sf::Vector2f(x_position, y_position));
		}

		void StickCollectionController::shuffleSticks()
		{
			std::random_device device;
			std::mt19937 random_engine(device());

			std::shuffle(sticks.begin(), sticks.end(), random_engine);
			updateStickPosition();
		}

		bool StickCollectionController::compareSticksByData(const Stick* a, const Stick* b) const
		{
			return a->data < b->data;
		}

		void StickCollectionController::processSortThreadState()
		{
			
			if (sort_thread.joinable() && isCollectionSorted())
			{
				sort_thread.join();
				sort_state = SortState::NOT_SORTING;
			}
		}


		void StickCollectionController::resetSticksColor()
		{
			for (int i = 0; i < sticks.size(); i++) sticks[i]->stick_view->setFillColor(collection_model->element_color);
		}

		void StickCollectionController::resetVariables()
		{
			number_of_comparisons = 0;
			number_of_array_access = 0;
		}

		void StickCollectionController::reset()
		{
			sort_state = SortState::NOT_SORTING;
			color_delay = 0;
			current_operation_delay = 0;
			if (sort_thread.joinable()) sort_thread.join();

			shuffleSticks();
			resetSticksColor();
			resetVariables();
		}

		void StickCollectionController::sortElements(SortType sort_type)
		{
			current_operation_delay = collection_model->operation_delay;
			this->sort_type = sort_type;
			sort_state = SortState::SORTING;
			color_delay = collection_model->initial_color_delay;

			switch (sort_type)
			{
			case Gameplay::Collection::SortType::BUBBLE_SORT:
				time_complexity = "O(n^2)";
				sort_thread = std::thread(&StickCollectionController::processBubbleSort, this);
				break;
			case Gameplay::Collection::SortType::INSERTION_SORT:
				time_complexity = "O(n^2)";
				sort_thread = std::thread(&StickCollectionController::processInsertionSort, this);
				break;
			case Gameplay::Collection::SortType::SELECTION_SORT:
				time_complexity = "O(n^2)";
				sort_thread = std::thread(&StickCollectionController::processSelectionSort, this);
				break;
			case Gameplay::Collection::SortType::MERGE_SORT:
				time_complexity = "O(n Log n)";
				sort_thread = std::thread(&StickCollectionController::processMergeSort, this);
				break;
			case Gameplay::Collection::SortType::QUICK_SORT:
				time_complexity = "O(n Log n)";
				sort_thread = std::thread(&StickCollectionController::processQuickSort, this);
				break;
			case Gameplay::Collection::SortType::RADIX_SORT:
				time_complexity = "ARSENAL";
				sort_thread = std::thread(&StickCollectionController::processRadixSort, this);
				break;
			}
		}

		bool StickCollectionController::isCollectionSorted()
		{
			for (int i = 1; i < sticks.size(); i++) if (sticks[i] < sticks[i - 1]) return false;
			return true;
		}

		void StickCollectionController::destroy()
		{
			current_operation_delay = 0;
			if (sort_thread.joinable()) sort_thread.join();

			for (int i = 0; i < sticks.size(); i++) delete(sticks[i]);
			sticks.clear();

			delete (collection_view);
			delete (collection_model);
		}

		void StickCollectionController::setCompletedColor()
		{
			for (int k = 0; k < sticks.size(); k++)
			{
				if (sort_state == SortState::NOT_SORTING) { break; }

				sticks[k]->stick_view->setFillColor(collection_model->element_color);
			}
			Sound::SoundService* sound = Global::ServiceLocator::getInstance()->getSoundService();

			for (int i = 0; i < sticks.size(); ++i)
			{
				if (sort_state == SortState::NOT_SORTING) { break; }

				sound->playSound(Sound::SoundType::COMPARE_SFX);
				sticks[i]->stick_view->setFillColor(collection_model->placement_position_element_color);

				// Delay to visualize the final color change
				std::this_thread::sleep_for(std::chrono::milliseconds(color_delay));

			}
			if (sort_state == SortState::SORTING)
			{
				sound->playSound(Sound::SoundType::SCREAM);
			}
		}

		void StickCollectionController::processBubbleSort()
		{
			Sound::SoundService* sound = Global::ServiceLocator::getInstance()->getSoundService();

			for (int i = 0;i < sticks.size()-1;i++)
			{
				if (sort_state == SortState::NOT_SORTING) break;
				bool swapped = false;
				
				for (int j = 0;j < sticks.size() - 1 - i;j++)
				{
					if (sort_state == SortState::NOT_SORTING) break;

					number_of_array_access+=2;number_of_comparisons++;
					sound->playSound(Sound::SoundType::COMPARE_SFX);

					sticks[j]->stick_view->setFillColor(collection_model->processing_element_color);
					std::cout << "Data of stick 1 is " << sticks[j]->data << std::endl;


					sticks[j + 1]->stick_view->setFillColor(collection_model->processing_element_color);
					std::cout << "Data of stick 2 is " << sticks[j+1]->data << std::endl;

					if (sticks[j]->data > sticks[j + 1]->data)
					{
						std::swap(sticks[j], sticks[j + 1]);
						std::cout<<"Swap happened\n";
						swapped = true;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));

					sticks[j]->stick_view->setFillColor(collection_model->element_color);
					sticks[j+1]->stick_view->setFillColor(collection_model->element_color);
					updateStickPosition();
				}
				std::cout << "Came out of J loop\n";


				sticks[sticks.size() - i-1]->stick_view->setFillColor(collection_model->placement_position_element_color);

				if (!swapped)
					break;
			}
			std::cout << "Came out of I loop\n";
			setCompletedColor();
		}

		void StickCollectionController::processInsertionSort()
		{
			Sound::SoundService* sound = Global::ServiceLocator::getInstance()->getSoundService();

			for (int i = 1; i < sticks.size(); ++i)
			{

				if (sort_state == SortState::NOT_SORTING) { break; }

				int j = i - 1;
				Stick* key = sticks[i];
				number_of_array_access++; // Access for key stick


				key->stick_view->setFillColor(collection_model->processing_element_color); // Current key is red

				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));

				while (j >= 0 && sticks[j]->data > key->data)
				{

					if (sort_state == SortState::NOT_SORTING) { break; }

					number_of_comparisons++;
					number_of_array_access++;

					sticks[j + 1] = sticks[j];
					number_of_array_access++; // Access for assigning sticks[j] to sticks[j + 1]
					sticks[j + 1]->stick_view->setFillColor(collection_model->processing_element_color); // Mark as being compared
					j--;
					sound->playSound(Sound::SoundType::COMPARE_SFX);
					updateStickPosition(); // Visual update

					std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));

					sticks[j + 2]->stick_view->setFillColor(collection_model->selected_element_color); // Mark as being compared

				}

				sticks[j + 1] = key;
				number_of_array_access++;
				sticks[j + 1]->stick_view->setFillColor(collection_model->temporary_processing_color); // Placed key is green indicating it's sorted
				sound->playSound(Sound::SoundType::COMPARE_SFX);
				updateStickPosition(); // Final visual update for this iteration
				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));
				sticks[j + 1]->stick_view->setFillColor(collection_model->selected_element_color); // Placed key is green indicating it's sorted
			}

			setCompletedColor();
		}

		void StickCollectionController::processSelectionSort()
		{
			Sound::SoundService* sound = Global::ServiceLocator::getInstance()->getSoundService();
			
			for (int i = 0;i < sticks.size()-1;i++)
			{
				if (sort_state == SortState::NOT_SORTING) { break; }

				int minIndex = i;
				sticks[i]->stick_view->setFillColor(collection_model->selected_element_color);

				for (int j = i+1;j < sticks.size();j++)
				{
					if (sort_state == SortState::NOT_SORTING) { break; }

					number_of_array_access += 2;
					number_of_comparisons++;

					sound->playSound(Sound::SoundType::COMPARE_SFX);
					
					sticks[j]->stick_view->setFillColor(collection_model->processing_element_color);
					std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));

					

					if (sticks[j]->data < sticks[minIndex]->data)
					{
						if(minIndex!=i)
							sticks[minIndex]->stick_view->setFillColor(collection_model->element_color);

						minIndex = j;
						sticks[minIndex]->stick_view->setFillColor(collection_model->temporary_processing_color);
					}
					else
						sticks[j]->stick_view->setFillColor(collection_model->element_color);
				}

				number_of_array_access += 3;//for swapping

				Stick* temp = sticks[i];
				sticks[i] = sticks[minIndex];
				sticks[minIndex] = temp;

				sticks[i]->stick_view->setFillColor(collection_model->placement_position_element_color);

				updateStickPosition();

				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));
			}
			setCompletedColor();
		}

		void StickCollectionController::processInPlaceMergeSort()
		{
			inPlaceMergeSort(0, sticks.size() - 1);

			setCompletedColor();
		}

		void StickCollectionController::processMergeSort()
		{
			outPlaceMergeSort(0, sticks.size() - 1);

			setCompletedColor();
		}

		void StickCollectionController::processQuickSort()
		{
			quickSort(0, sticks.size() - 1);

			setCompletedColor();
		}

		void StickCollectionController::processRadixSort()
		{
			radixSort();

			setCompletedColor();
		}

		void StickCollectionController::inPlaceMerge(int left, int mid, int right)
		{
			Sound::SoundService* sound = Global::ServiceLocator::getInstance()->getSoundService();

			int i = left;
			int j = mid + 1;
			int k = left;

			while (i <= mid && j <= right)
			{
				if (sticks[i]->data <= sticks[j]->data)
				{
					i++;
					k++;

					number_of_array_access += 2;
					number_of_comparisons++;
				}
				else
				{
					Stick* temp = sticks[j];
					for (int l = j; l > k; l--)
					{
						sticks[l] = sticks[l - 1];
						number_of_array_access += 2;
					}
					sticks[k] = temp;number_of_array_access++;
					i++;
					mid++;
					j++;
					k++;

					sound->playSound(Sound::SoundType::COMPARE_SFX);
					updateStickPosition();

					std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));
				}
			}
		}

		void StickCollectionController::inPlaceMergeSort(int left, int right)
		{
			if (left < right)
			{
				int mid = left + (right - left) / 2;
				inPlaceMergeSort(left, mid);
				inPlaceMergeSort(mid + 1, right);
				inPlaceMerge(left, mid, right);
			}
			else
				return;
		}

		void StickCollectionController::outPlaceMergeSort(int left, int right)
		{
			
			if (left < right)
			{
				int mid = left + (right - left) / 2;
				outPlaceMergeSort(left, mid);
				outPlaceMergeSort(mid + 1, right);
				outPlaceMerge(left, mid, right);
			}
		}

		void StickCollectionController::outPlaceMerge(int left, int mid, int right)
		{
			Sound::SoundService* sound = Global::ServiceLocator::getInstance()->getSoundService();
			
			int size = right - left + 1;
			std::vector<Stick*> temp(right - left + 1);

			int i = left;int j = mid + 1;int k = 0;

			// Copy elements to the temporary array
			for (int index = left; index <= right; ++index) {
				temp[k++] = sticks[index];
				number_of_array_access += 2;//two
				sticks[index]->stick_view->setFillColor(collection_model->temporary_processing_color);
				updateStickPosition();
			}

			i = 0;  // Start of the first half in temp
			j = mid - left + 1;  // Start of the second half in temp
			k = left;  // Start position in the original array to merge back

			// Merge elements back to the original array from temp
			while (i < mid - left + 1 && j < temp.size()) {
				if (temp[i]->data <= temp[j]->data) {
					sticks[k] = temp[i++];
				}
				else {
					sticks[k] = temp[j++];
				}
				number_of_array_access += 2;number_of_comparisons++;

				sound->playSound(Sound::SoundType::COMPARE_SFX);
				sticks[k]->stick_view->setFillColor(collection_model->processing_element_color);
				updateStickPosition();  // Immediate update after assignment
				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));

				k++;

			}

			// Handle remaining elements from both halves
			while (i < mid - left + 1 || j < temp.size()) {
				if (i < mid - left + 1) {
					sticks[k] = temp[i++];
				}
				else {
					sticks[k] = temp[j++];
				}
				number_of_array_access += 2;number_of_comparisons++;

				sound->playSound(Sound::SoundType::COMPARE_SFX);
				sticks[k]->stick_view->setFillColor(collection_model->processing_element_color);
				updateStickPosition();  // Immediate update
				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));

				k++;
			}


		}

		void StickCollectionController::quickSort(int low, int high)
		{
			if (low < high) 
			{
				int pivot = partition(low, high);
				quickSort(low, pivot - 1);
				quickSort(pivot + 1, high);
			}

			for (int i = low; i <= high; i++) 
			{
				sticks[i]->stick_view->setFillColor(collection_model->placement_position_element_color);
				updateStickPosition();
			}
		}

		int StickCollectionController::partition(int low, int high)
		{
			Sound::SoundService* sound = Global::ServiceLocator::getInstance()->getSoundService();
			
			int pivot_data = sticks[high]->data;
			sticks[high]->stick_view->setFillColor(collection_model->selected_element_color);
			
			int swap_marker = low - 1;

			for (int j = low;j < high;j++)
			{
				sticks[j]->stick_view->setFillColor(collection_model->processing_element_color);
				number_of_array_access += 2;
				number_of_comparisons++;
				
				if (sticks[j]->data < pivot_data)
				{
					swap_marker++;

					Stick* temp = sticks[swap_marker];
					sticks[swap_marker] = sticks[j];
					sticks[j] = temp;

					number_of_array_access += 3;
					sound->playSound(Sound::SoundType::COMPARE_SFX);

					updateStickPosition();
					std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay));
				}

				sticks[j]->stick_view->setFillColor(collection_model->element_color);
			}

			Stick* temp = sticks[swap_marker + 1];
			sticks[swap_marker + 1] = sticks[high];
			sticks[high] = temp;

			number_of_array_access += 3;

			updateStickPosition();

			return swap_marker + 1;
		}

		void StickCollectionController::radixSort()
		{
			int max = sticks[0]->data;

			for (int i = 1;i < sticks.size();i++)
			{
				if (sticks[i]->data > max)
				{
					max = sticks[i]->data;
				}
			}

			for (int exp = 1;max / exp > 0;exp *= 10)
			{
				countSort(exp);
			}
		}

		void StickCollectionController::countSort(int exponent)
		{
			Sound::SoundService* sound = Global::ServiceLocator::getInstance()->getSoundService();

			int count[10] = { 0 };
			std::vector<Stick*> output(sticks.size());

			for (int i = 0;i < sticks.size();i++)//making the count array
			{
				int digit = (sticks[i]->data / exponent) % 10;
				count[digit]++;

				number_of_array_access++;

				sticks[i]->stick_view->setFillColor(collection_model->processing_element_color);
				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay / 2)); // Delay for visual processing
				sticks[i]->stick_view->setFillColor(collection_model->element_color);  // Reset color after processing
			}

			for (int i = 1;i < 10;i++)//making count cumulatibe
			{
				count[i] += count[i - 1];
			}

			for (int i = sticks.size() - 1;i >= 0;i--)//putting the elements in the output array
			{
				int digit = (sticks[i]->data / exponent) % 10;

				output[count[digit] - 1] = sticks[i];
				output[count[digit] - 1]->stick_view->setFillColor(collection_model->temporary_processing_color);
				count[digit]--;
				number_of_array_access++;
			}

			for (int i = 0;i < sticks.size();i++)//copying the output array to the original array
			{
				sticks[i] = output[i];

				sticks[i]->stick_view->setFillColor(collection_model->placement_position_element_color);  // Final sorted color for this digit
				updateStickPosition(i);
				std::this_thread::sleep_for(std::chrono::milliseconds(current_operation_delay)); // Delay to observe final sorting stat
			}
		}

		SortType StickCollectionController::getSortType() { return sort_type; }

		int StickCollectionController::getNumberOfComparisons() { return number_of_comparisons; }

		int StickCollectionController::getNumberOfArrayAccess() { return number_of_array_access; }

		int StickCollectionController::getNumberOfSticks() { return collection_model->number_of_elements; }

		int StickCollectionController::getDelayMilliseconds() { return current_operation_delay; }

		sf::String StickCollectionController::getTimeComplexity() { return time_complexity; }
	}
}


