/*
 * combineKmers.cpp
 * Takes the union of kmer counts generated by dsk
 *
 */


//
// Consider transforming bases to a more compact representation
//
// Just use sample index vs. use full sample name
//
// TODO read dsk output directly

#include "combineKmers.hpp"

int main (int argc, char *argv[])
{
   // Program description
   std::cerr << "combineKmers: basic algorithm to combine kmer counts across samples\n";

   // Do parsing and checking of command line params
   // If no input options, give quick usage rather than full help
   boost::program_options::variables_map vm;
   if (argc == 1)
   {
      std::cerr << "Usage: combineKmers -s samples.txt -o all_kmers --min_samples 2\n\n"
         << "For full option details run combineKmers -h\n";
      return 0;
   }
   else if (parseCommandLine(argc, argv, vm))
   {
      return 1;
   }

   // Read in list of sample kmer files and their names
   std::vector<std::tuple<std::string, std::string> > samples = readSamples(vm["samples"].as<std::string>());
   std::unordered_map<int, std::string> sample_names;
   size_t min_samples = checkMin(samples.size(), vm["min_samples"].as<int>());

   // Open the output file before counting kmers
   ogzstream out_file((vm["output"].as<std::string>() + ".gz").c_str());

   // Map to store kmers
   std::unordered_map<std::string, std::vector<std::tuple<int, int>>> kmer_union;

   // Add kmers to map
   std::cerr << "Reading and mapping kmers..." << std::endl;
   for (unsigned int i = 0; i < samples.size(); ++i)
   {
      std::string sample_name, filename;
      std::tie(sample_name, filename) = samples[i];

      // Rather than storing lots of copies of sample names as strings, just
      // store hash keys
      sample_names[i] = sample_name;

      std::ifstream kmer_counts(filename.c_str());

      if (kmer_counts)
      {
         std::cerr << "File " << i + 1 << "/" << samples.size() << "\r";
         std::cerr.flush();

         std::string kmer, abundance;
         while (kmer_counts)
         {
            kmer_counts >> kmer >> abundance;

            kmer_union[kmer].push_back(std::make_tuple(i, stoi(abundance)));
         }
      }
      else
      {
         std::cerr << "Could not open " + filename << std::endl;
         std::cerr << "Skipping..." << std::endl;
      }
   }
   std::cerr << std::endl;

   // Print results
   std::cerr << "Printing union of kmers" << std::endl;
   for(auto kmer_it = kmer_union.cbegin(); kmer_it != kmer_union.cend(); ++kmer_it)
   {
      if (kmer_it->second.size() >= min_samples)
      {
         out_file << kmer_it->first << " ";
         for (auto sample_it = kmer_it->second.cbegin(); sample_it != kmer_it->second.cend(); ++sample_it)
         {
            int sample, abundance;
            std::tie (sample, abundance) = *sample_it;

            out_file << sample_names[sample] + ":" + std::to_string(abundance);
         }
         out_file << std::endl;
      }
   }

   std::cerr << "Done." << std::endl;

   return(0);
}

