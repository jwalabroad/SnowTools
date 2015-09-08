#ifndef SNOWTOOLS_BREAKPOINT_H__
#define SNOWTOOLS_BREAKPOINT_H__

#include <cstdlib>
#include <string>
#include <unordered_map>
#include "htslib/faidx.h"

#include "SnowTools/GenomicRegion.h"
#include "SnowTools/GenomicRegionCollection.h"
#include "SnowTools/DiscordantCluster.h"
#include "SnowTools/BamRead.h"
#include "SnowTools/BWAWrapper.h"

#include "SnowTools/STCoverage.h"

namespace SnowTools {

  // forward declares
struct BreakPoint;

typedef std::vector<BreakPoint> BPVec;
typedef std::unordered_map<std::string, size_t> PON;

 struct BreakEnd {
   
   BreakEnd() {}

   BreakEnd(const GenomicRegion& g, int mq, const std::string & chr_n);
   
   BreakEnd(const BamRead& b);

   std::string id;
   std::string chr_name;
   GenomicRegion gr;
   int mapq = -1;
   int cpos = -1;
   int nm = -1;
   int matchlen = -1;
   int tsplit = -1;
   int nsplit = -1;
   int sub_n = -1;
   bool local;
   double n_af = -1;
   double t_af = -1;

 };
 
 struct BreakPoint {
   
   static std::string header() { 
     return "chr1\tpos1\tstrand1\tchr2\tpos2\tstrand2\tref\talt\tspan\tmapq1\tmapq2\tnsplit\ttsplit\tsubn1\tsubn2\tndisc\ttdisc\tdisc_mapq1\tdisc_mapq2\tncigar\ttcigar\thomology\tinsertion\tcontig\tnumalign\tconfidence\tevidence\tquality\tsecondary_alignment\tsomatic_score\tpon_samples\trepeat_seq\tnormal_cov\ttumor_cov\tnormal_allelic_fraction\ttumor_allelic_fraction\tgraylist\tDBSNP\treads"; 
   }

   std::string ref;
   std::string alt;

   // the evidebce per break-end
   BreakEnd b1, b2;
   
   // reads spanning this breakpoint
   BamReadVector reads;

   // allelic fraction
   double af_t = -1;
   double af_n = -1;
   
   // discordant reads supporting this aseembly bp
   DiscordantCluster dc;
   
   int quality = 0;

   // string of read names concatenated together
   std::string read_names;
   
   // total coverage at that position
   int tcov = 0;
   int ncov = 0;
   
   // total coverage supporting the variant at that position
   int tcov_support = 0;
   int ncov_support = 0;

   bool secondary = false;
   
   // DBsnp 
   std::string rs = "";
   
   std::string seq;
   
   std::string cname;
   
   std::string insertion = "";
   std::string homology = "";
   
   std::string repeat_seq = "";
   
   int tcigar = 0;
   int ncigar = 0;
   
   double somatic_score = 0;
   
   int pon = 0;
   
   int nsplit = 0;
   int tsplit = 0;
   
   int num_align = 0;

   std::string evidence = "";
   std::string confidence = "";
   
   bool isindel = false;
   
   bool blacklist = false;
   
   /** Construct a breakpoint from a cluster of discordant reads
    */
   BreakPoint(const DiscordantCluster& tdc, const BWAWrapper * bwa);
     
   BreakPoint() {}
   
   BreakPoint(const std::string &line, bam_hdr_t* h);
   
   /*! Return a string with information useful for printing at the 
    * command line as Snowman runs 
    * @return string with minimal information about the BreakPoint.
    */
   std::string toPrintString() const;
   
   //static void readPON(std::string &file, std::unique_ptr<PON> &pmap);

   /** Retrieve the reference sequence at a breakpoint and determine if 
    * it lands on a repeat */
   //void repeatFilter(faidx_t * f);
   void __combine_with_discordant_cluster(DiscordantClusterMap& dmap);
   
   /*! @function determine if the breakpoint has split read support
    * @param reference to a vector of read smart pointers that have been aligned to a contig
    * @discussion Note: will cause an error if the AL tag not filled in for the reads. 
    * The AL tag is filled in by AlignedContig::alignReadsToContigs.
    */
   void splitCoverage(BamReadVector &bav);
   
   /*! Determines if the BreakPoint overlays a blacklisted region. If 
    * and overlap is found, sets the blacklist bool to true.
    *
    * Note that currently this only is set for the pos1 of indels.
    * If the BreakPoint object is not an indel, no action is taken. 
    * @param grm An interval tree map created from a BED file containing blacklist regions
    */
   void checkBlacklist(GRC &grv);
   
   /*! Score a breakpoint with a QUAL score, and as somatic or germline
    */
   void scoreBreakpoint();
   
   /*! Compute the allelic fraction (tumor and normal) for this BreakPoint.
    *
   * The allelic fraction is computed by taking the base-pair level coverage
   * as the denominator, and the max of number of split reads and number of 
   * cigar supporting reads as the numerator. Note that because, theoretically
   * but rarely, the number of split reads could be > 0 while the bp-level coverage
   * at a variant could be exactly zero. This is because unmapped reads could be called split
   * reads but are not counted in the coverage calculation. In such a case, the allelic fraction is
   * set to -1. By the same argument, the allelic fraction could rarely be > 1.
   * @param t_cov Base-pair level Coverage object, with coverage for all reads from Tumor bam(s).
   * @param n_cov Base-pair level Coverage object, with coverage for all reads from Normal bam(s).
   */
  void addAllelicFraction(STCoverage * t_cov, STCoverage * n_cov);
  
  /*! @function get the span of the breakpoints (in bp). -1 for interchrom
   * @return int distance between breakpoints
   */
   int getSpan() const;

   /*! @function check the breakpoint against a panel of normals
    * @param Panel of normals hash
    * @return number of normal samples with this variant
    */
   //int checkPon(std::unique_ptr<PON> &p);
   
   /*! @function get a unique string representation of this breakpoint.
    * Format for indel is chr_breakpos_type (eg. 0_134134_I)
    * @return string with breakpoint info
    */
   std::string getHashString() const;
   
   bool hasMinimal() const;
   
   bool sameBreak(BreakPoint &bp) const;
   
   void order();
   
   bool isEmpty() const { return (b1.gr.pos1 == 0 && b2.gr.pos1 == 0); }
   
   std::string toFileString(bool noreads = false);
   
   bool hasDiscordant() const;
   
   bool operator==(const BreakPoint& bp) const;
   
   // define how to sort these 
   bool operator < (const BreakPoint& bp) const { 
     
     if (b1.gr < bp.b1.gr)
       return true;
     else if (bp.b1.gr < b1.gr)
       return false;
     
     if (b2.gr < bp.b2.gr)
       return true;
     else if (bp.b2.gr < b2.gr)
       return false;
     
     if (nsplit > bp.nsplit)
       return true;
     else if (nsplit < bp.nsplit)
       return false;
     
     if (tsplit > bp.tsplit)
       return true;
     else if (tsplit < bp.tsplit)
       return false;
     
     if (tsplit > bp.tsplit)
       return true;
     else if (tsplit < bp.tsplit)
       return false;
     
     if (dc.ncount > bp.dc.ncount)
       return true;
     else if (dc.ncount < bp.dc.ncount)
       return false;
     
     if (dc.tcount > bp.dc.tcount)
       return true;
     else if (dc.tcount < bp.dc.tcount)
       return false;
     
     if (cname > bp.cname)
       return true;
     else if (cname < bp.cname)
       return false;
     
     return false;
  }

   friend std::ostream& operator<<(std::ostream& out, const BreakPoint& bp);
   
   void __score_dscrd();
   void __score_assembly_only();
   void __score_assembly_dscrd();
   void __score_indel();
   std::string __format_readname_string();
   void __set_homologies_insertions();
   void __set_evidence();
   void __set_allelic_fraction();
   bool valid() const;
   
   double __sv_is_somatic() const;
   double __indel_is_somatic() const;

   void setRefAlt(faidx_t * main_findex, faidx_t * viral_findex);

};

}

#endif
