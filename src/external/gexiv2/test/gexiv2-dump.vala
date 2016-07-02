/*
 * gexiv2-dump.vala
 *
 * Author(s)
 *  Jim Nelson <jim@yorba.org>
 *
 * This is free software. See COPYING for details.
 */

int main(string[] args) {
    if (args.length < 2 || ("--help" in args) || ("-h" in args)) {
        usage();
        
        return 1;
    }
    
    // skip args[0]
    foreach (string filename in args[1:args.length]) {
        try {
            GExiv2.Metadata metadata = new GExiv2.Metadata();
            metadata.open_path(filename);
            
            dump_tags(metadata, metadata.get_exif_tags());
            dump_tags(metadata, metadata.get_iptc_tags());
            dump_tags(metadata, metadata.get_xmp_tags());
        } catch (Error err) {
            stderr.printf("Unable to dump metadata for %s: %s\n", filename, err.message);
        }
    }
    
    return 0;
}

void usage() {
    stdout.printf("usage: gexiv2-dump FILE...\n\n");
}

void dump_tags(GExiv2.Metadata metadata, string[] tags) throws Error {
    foreach (string tag in tags) {
        stdout.printf("%s%*s%s\n",
            tag,
            (64 - tag.length).clamp(1, 64), "",
            metadata.get_tag_interpreted_string(tag)
        );
    }
}

